#pragma once
#include <JuceHeader.h>
#include <vector>
#include <cmath>

struct Params
{
    float inp = 0, hp = 30, low = 0, mid = 0, midf = 1000, high = 0, drive = .1f, thr = -18, ratio = 3;
    float pre = 10, color = 12000, rvTime = 1.5f, depth = .7f, mix = .36f, bal = .5f, fb = .3f, beats = .5f;
    float dlLp = 8000, out = 0, ceil = -1;
    bool rvOn = true, dlOn = true, autoTempo = true;
    int rvType = 0, dlType = 0;
};

struct Ring
{
    std::vector<float> b; int w = 0;
    void init(int n) { b.assign((size_t) juce::jmax(8, n), 0.f); w = 0; }
    void push(float x) { b[(size_t) w] = x; if (++w >= (int) b.size()) w = 0; }
    float read(float d) const
    {
        const int n = (int) b.size();
        float p = (float) w - d;
        while (p < 0) p += (float) n;
        int i0 = (int) p; float f = p - (float) i0; int i1 = i0 + 1;
        if (i0 >= n) i0 -= n;
        if (i1 >= n) i1 -= n;
        return b[(size_t) i0] * (1.f - f) + b[(size_t) i1] * f;
    }
};

class Engine
{
public:
    using Co = juce::dsp::IIR::Coefficients<float>;
    using Filt = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, Co>;

    Engine()
    {
        hp.state = Co::makeHighPass(44100.0, 30.0f);
        lo.state = Co::makeLowShelf(44100.0, 120.0f, 0.7f, 1.0f);
        md.state = Co::makePeakFilter(44100.0, 1000.0f, 1.0f, 1.0f);
        hi.state = Co::makeHighShelf(44100.0, 8000.0f, 0.7f, 1.0f);
    }

    void prepare(double sr, int maxBlock)
    {
        sampleRate = sr;
        juce::dsp::ProcessSpec s { sr, (juce::uint32) maxBlock, 2 };
        hp.prepare(s); lo.prepare(s); md.prepare(s); hi.prepare(s); comp.prepare(s); limiter.prepare(s);
        reverb.setSampleRate(sr); reverb.reset();
        preL.init((int)(sr * .12) + 4); preR.init((int)(sr * .12) + 4);
        dly[0].init((int)(sr * 2.1)); dly[1].init((int)(sr * 2.1));
        lp[0] = lp[1] = 0.f;
        rv.setSize(2, maxBlock);
        lastHp = lastLo = lastMid = lastMidF = lastHi = -999.f;
    }

    void process(juce::AudioBuffer<float>& buf, const Params& p, double bpm)
    {
        const int n = buf.getNumSamples();
        if (n <= 0 || buf.getNumChannels() < 2) return;
        if (rv.getNumSamples() < n) rv.setSize(2, n, false, false, true);
        if (bpm < 30.0) bpm = 120.0;

        buf.applyGain(juce::Decibels::decibelsToGain(p.inp));

        if (p.hp != lastHp)   { *hp.state = *Co::makeHighPass(sampleRate, p.hp); lastHp = p.hp; }
        if (p.low != lastLo)  { *lo.state = *Co::makeLowShelf(sampleRate, 120.0f, .7f, juce::Decibels::decibelsToGain(p.low)); lastLo = p.low; }
        if (p.mid != lastMid || p.midf != lastMidF)
        { *md.state = *Co::makePeakFilter(sampleRate, p.midf, 1.0f, juce::Decibels::decibelsToGain(p.mid)); lastMid = p.mid; lastMidF = p.midf; }
        if (p.high != lastHi) { *hi.state = *Co::makeHighShelf(sampleRate, 8000.0f, .7f, juce::Decibels::decibelsToGain(p.high)); lastHi = p.high; }

        juce::dsp::AudioBlock<float> blk(buf);
        juce::dsp::ProcessContextReplacing<float> ctx(blk);
        hp.process(ctx); lo.process(ctx); md.process(ctx); hi.process(ctx);

        float* L = buf.getWritePointer(0);
        float* R = buf.getWritePointer(1);

        if (p.drive > 0.001f)
        {
            const float k = 1.f + p.drive * 8.f, nrm = 1.f / std::tanh(k);
            for (int i = 0; i < n; ++i) { L[i] = std::tanh(k * L[i]) * nrm; R[i] = std::tanh(k * R[i]) * nrm; }
        }

        comp.setThreshold(p.thr); comp.setRatio(p.ratio); comp.setAttack(10.f);
        comp.setRelease(p.autoTempo ? juce::jlimit(50.f, 400.f, (float)(60000.0 / bpm * .5)) : 150.f);
        comp.process(ctx);

        // ---- Reverb (pre-delay + Freeverb) ----
        const float rt = p.autoTempo ? juce::jmin(p.rvTime, juce::jmax(.6f, (float)(60.0 / bpm * 4.0))) : p.rvTime;
        static const float roomScale[4] = { 1.0f, 1.08f, .92f, .7f }, dampOff[4] = { 0.f, .1f, -.05f, .2f };
        juce::Reverb::Parameters rp;
        rp.roomSize = juce::jlimit(0.f, .98f, ((rt - .3f) / 4.7f * .6f + .35f) * roomScale[p.rvType]);
        rp.damping = juce::jlimit(0.f, 1.f, 1.f - (p.color - 1500.f) / 14500.f + dampOff[p.rvType]);
        rp.wetLevel = 1.f; rp.dryLevel = 0.f; rp.width = 1.f; rp.freezeMode = 0.f;
        reverb.setParameters(rp);

        float* rl = rv.getWritePointer(0);
        float* rr = rv.getWritePointer(1);
        const float preS = juce::jmax(1.f, p.pre * .001f * (float) sampleRate);
        for (int i = 0; i < n; ++i)
        {
            preL.push(L[i]); preR.push(R[i]);
            rl[i] = preL.read(preS); rr[i] = preR.read(preS);
        }
        reverb.processStereo(rl, rr, n);

        // ---- Delay (tempo-synced) + mix ----
        const float rs = juce::jmin(1.f, 2.f * (1.f - p.bal)), ds = juce::jmin(1.f, 2.f * p.bal);
        const float gR = p.rvOn ? p.depth * p.mix * rs * 1.2f : 0.f;
        const float gD = p.dlOn ? p.mix * ds * .9f : 0.f;
        const float dS = (float) juce::jmin(1.9, 60.0 / bpm * (double) p.beats) * (float) sampleRate;
        static const float cap[3] = { 4500.f, 12000.f, 2500.f };
        const float fc = juce::jmin(p.dlLp, cap[p.dlType]);
        const float a = 1.f - std::exp(-juce::MathConstants<float>::twoPi * fc / (float) sampleRate);
        float* ch[2] = { L, R }; float* rw[2] = { rl, rr };
        for (int i = 0; i < n; ++i)
            for (int c = 0; c < 2; ++c)
            {
                const float x = ch[c][i];
                const float del = dly[c].read(dS);
                lp[c] += a * (del - lp[c]);
                dly[c].push(x + p.fb * lp[c]);
                ch[c][i] = x + rw[c][i] * gR + lp[c] * gD;
            }

        buf.applyGain(juce::Decibels::decibelsToGain(p.out));
        limiter.setThreshold(p.ceil); limiter.setRelease(100.f);
        limiter.process(ctx);
    }

    // Auto tempo detection from audio (onset autocorrelation). Returns BPM or -1.
    static double detectBpm(const juce::AudioBuffer<float>& b, double sr)
    {
        const int h = juce::jmax(1, (int) std::lround(sr / 200.0));
        const double fps = sr / (double) h;
        const int N = juce::jmin(b.getNumSamples() / h, 20000);
        if (N < 400) return -1.0;
        const float* a = b.getReadPointer(0);
        const float* c = b.getReadPointer(b.getNumChannels() > 1 ? 1 : 0);
        std::vector<float> e((size_t) N), o((size_t) N, 0.f);
        float pv = 0.f;
        for (int i = 0; i < N; ++i)
        {
            double s = 0, t = 0;
            for (int j = 0; j < h; ++j)
            {
                const int k = i * h + j;
                const float m = .5f * (a[k] + c[k]); const float d = m - pv; pv = m;
                s += (double) m * m; t += (double) d * d;
            }
            e[(size_t) i] = (float)(std::log(1.0 + 300.0 * s / h) + std::log(1.0 + 1000.0 * t / h));
        }
        double sm = 0;
        for (int i = 1; i < N; ++i) { o[(size_t) i] = juce::jmax(0.f, e[(size_t) i] - e[(size_t) i - 1]); sm += o[(size_t) i]; }
        sm /= N;
        for (int i = 0; i < N; ++i) o[(size_t) i] = juce::jmax(0.f, o[(size_t) i] - (float) sm);

        auto g = [&](double Lg)
        {
            const int li = (int) Lg; const double f = Lg - li; double s = 0; int cnt = 0;
            for (int i = 0; i + li + 1 < N; ++i) { s += (double) o[(size_t) i] * ((double) o[(size_t)(i + li)] * (1 - f) + (double) o[(size_t)(i + li + 1)] * f); ++cnt; }
            return cnt ? s / cnt : 0.0;
        };
        double best = 0, bs = 0;
        for (double t = 60.0; t <= 200.0; t += .25)
        {
            const double Lg = fps * 60.0 / t;
            const double prior = std::exp(-.5 * std::pow(std::log2(t / 105.0) / .85, 2.0));
            const double v = (g(Lg) + .5 * g(2 * Lg) + .25 * g(4 * Lg)) * prior;
            if (v > bs) { bs = v; best = t; }
        }
        if (best <= 0) return -1.0;
        if (std::abs(best - std::round(best)) < .2) best = std::round(best);
        return best;
    }

private:
    double sampleRate = 44100.0;
    Filt hp, lo, md, hi;
    juce::dsp::Compressor<float> comp;
    juce::dsp::Limiter<float> limiter;
    juce::Reverb reverb;
    Ring preL, preR, dly[2];
    float lp[2] { 0.f, 0.f };
    juce::AudioBuffer<float> rv;
    float lastHp = -999.f, lastLo = -999.f, lastMid = -999.f, lastMidF = -999.f, lastHi = -999.f;
};
