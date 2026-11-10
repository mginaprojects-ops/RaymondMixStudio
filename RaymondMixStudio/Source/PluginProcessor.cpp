#include "PluginProcessor.h"
#include "PluginEditor.h"

static int divIndex(float beats)
{
    static const float b[5] = { .25f, .5f, .75f, 1.f, 2.f };
    int best = 1; float bd = 1e9f;
    for (int i = 0; i < 5; ++i) if (std::abs(b[i] - beats) < bd) { bd = std::abs(b[i] - beats); best = i; }
    return best;
}

juce::AudioProcessorValueTreeState::ParameterLayout RaymondProcessor::makeLayout()
{
    using F = juce::AudioParameterFloat;
    using NR = juce::NormalisableRange<float>;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> v;
    auto f = [&](const char* id, const char* name, float a, float b, float d, float skew = 1.f)
    { v.push_back(std::make_unique<F>(juce::ParameterID { id, 1 }, name, NR(a, b, 0.f, skew), d)); };
    auto bl = [&](const char* id, const char* name, bool d)
    { v.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID { id, 1 }, name, d)); };
    auto ch = [&](const char* id, const char* name, juce::StringArray items, int d)
    { v.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { id, 1 }, name, items, d)); };

    f("inp", "Input", -12, 12, 0);
    f("hp", "High-pass", 20, 300, 30, .5f);
    f("low", "Low", -12, 12, 0);
    f("mid", "Mid", -12, 12, 0);
    f("midf", "Mid Freq", 200, 5000, 1000, .4f);
    f("high", "High", -12, 12, 0);
    f("drive", "Saturation", 0, 1, .15f);
    f("thr", "Comp Threshold", -40, 0, -18);
    f("ratio", "Comp Ratio", 1, 12, 3);
    f("pre", "Pre-delay", 0, 100, 10);
    f("color", "Reverb Color", 1500, 16000, 12000, .5f);
    f("rvTime", "Reverb Time", .3f, 5, 1.6f, .6f);
    f("depth", "Depth", 0, 1, .7f);
    f("mix", "Mix", 0, 1, .36f);
    f("bal", "Reverb/Delay Balance", 0, 1, .5f);
    f("fb", "Feedback", 0, .85f, .3f);
    ch("div", "Delay Rate", { "1/16", "1/8", "1/8.", "1/4", "1/2" }, 1);
    f("dlLp", "Delay Tone", 1500, 16000, 8000, .5f);
    f("out", "Output", -12, 12, 0);
    f("ceil", "Ceiling", -6, 0, -1);
    f("bpm", "BPM (manual / file)", 40, 220, 120);
    bl("rvOn", "Reverb On", true);
    bl("dlOn", "Delay On", true);
    bl("autoTempo", "Auto Tempo", true);
    bl("bypass", "Bypass", false);
    ch("rvType", "Reverb Type", { "GLASS", "HALL", "PLATE", "ROOM" }, 0);
    ch("dlType", "Delay Type", { "STEREO TAPE", "DIGITAL", "DARK TAPE" }, 0);
    return { std::make_move_iterator(v.begin()), std::make_move_iterator(v.end()) };
}

RaymondProcessor::RaymondProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "STATE", makeLayout())
{
    setCurrentProgram(0);
}

Params RaymondProcessor::gather()
{
    auto g = [this](const char* id) { return apvts.getRawParameterValue(id)->load(); };
    static const float beats[5] = { .25f, .5f, .75f, 1.f, 2.f };
    Params p;
    p.inp = g("inp"); p.hp = g("hp"); p.low = g("low"); p.mid = g("mid"); p.midf = g("midf"); p.high = g("high");
    p.drive = g("drive"); p.thr = g("thr"); p.ratio = g("ratio"); p.pre = g("pre"); p.color = g("color");
    p.rvTime = g("rvTime"); p.depth = g("depth"); p.mix = g("mix"); p.bal = g("bal"); p.fb = g("fb");
    p.beats = beats[juce::jlimit(0, 4, (int) g("div"))];
    p.dlLp = g("dlLp"); p.out = g("out"); p.ceil = g("ceil");
    p.rvOn = g("rvOn") > .5f; p.dlOn = g("dlOn") > .5f; p.autoTempo = g("autoTempo") > .5f;
    p.rvType = juce::jlimit(0, 3, (int) g("rvType")); p.dlType = juce::jlimit(0, 2, (int) g("dlType"));
    return p;
}

void RaymondProcessor::prepareToPlay(double sr, int block) { engine.prepare(sr, block); }

bool RaymondProcessor::isBusesLayoutSupported(const BusesLayout& l) const
{
    return l.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void RaymondProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals nd;
    const int n = buffer.getNumSamples();
    for (int c = 2; c < buffer.getNumChannels(); ++c) buffer.clear(c, 0, n);
    if (buffer.getNumChannels() < 2 || n <= 0) return;

    const float il = juce::jmax(buffer.getMagnitude(0, 0, n), buffer.getMagnitude(1, 0, n));
    inLevel.store(il);

    double bpm = apvts.getRawParameterValue("bpm")->load();
    int src = 0;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto b = pos->getBpm())
                if (*b > 20.0) { bpm = *b; src = 1; }   // DELAY SENSES THE DAW TEMPO
    currentBpm.store(bpm); bpmSource.store(src);

    if (apvts.getRawParameterValue("bypass")->load() > .5f) { outLevel.store(il); return; }

    engine.process(buffer, gather(), bpm);
    outLevel.store(juce::jmax(buffer.getMagnitude(0, 0, n), buffer.getMagnitude(1, 0, n)));
}

void RaymondProcessor::setCurrentProgram(int index)
{
    program = juce::jlimit(0, kNumPresets - 1, index);
    for (int i = 0; i < 16; ++i)
    {
        float val = kPresets[program].v[i];
        if (i == 13) val = (float) divIndex(val);
        if (auto* prm = apvts.getParameter(kIds[i]))
            prm->setValueNotifyingHost(prm->convertTo0to1(val));
    }
}

void RaymondProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary(*xml, dest);
}

void RaymondProcessor::setStateInformation(const void* data, int size)
{
    if (auto xml = getXmlFromBinary(data, size))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

double RaymondProcessor::loadAudio(const juce::File& f)
{
    juce::AudioFormatManager fm; fm.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> r(fm.createReaderFor(f));
    if (r == nullptr || r->lengthInSamples <= 0) return -1.0;
    const int len = (int) r->lengthInSamples;
    loaded.setSize(2, len);
    r->read(&loaded, 0, len, 0, true, true);
    if (r->numChannels == 1) loaded.copyFrom(1, 0, loaded, 0, 0, len);
    loadedSr = r->sampleRate;
    const double b = Engine::detectBpm(loaded, loadedSr);   // AUTO TEMPO DETECTION ON IMPORT
    if (b > 0)
        if (auto* prm = apvts.getParameter("bpm")) prm->setValueNotifyingHost(prm->convertTo0to1((float) b));
    return b;
}

bool RaymondProcessor::renderToWav(const juce::File& out)
{
    if (!hasAudio()) return false;
    const int tail = (int)(loadedSr * 3.0), total = loaded.getNumSamples() + tail;
    juce::AudioBuffer<float> o(2, total);
    o.clear();
    o.copyFrom(0, 0, loaded, 0, 0, loaded.getNumSamples());
    o.copyFrom(1, 0, loaded, 1, 0, loaded.getNumSamples());

    Engine eng; eng.prepare(loadedSr, 512);
    const Params p = gather();
    const double bpm = apvts.getRawParameterValue("bpm")->load();
    for (int pos = 0; pos < total; pos += 512)
    {
        const int len = juce::jmin(512, total - pos);
        float* ptrs[2] = { o.getWritePointer(0, pos), o.getWritePointer(1, pos) };
        juce::AudioBuffer<float> view(ptrs, 2, len);
        eng.process(view, p, bpm);
    }

    out.deleteFile();
    std::unique_ptr<juce::FileOutputStream> os(out.createOutputStream());
    if (os == nullptr) return false;
    juce::WavAudioFormat wav;
    std::unique_ptr<juce::AudioFormatWriter> w(wav.createWriterFor(os.get(), loadedSr, 2, 24, {}, 0));
    if (w == nullptr) return false;
    os.release();
    return w->writeFromAudioSampleBuffer(o, 0, total);
}

juce::AudioProcessorEditor* RaymondProcessor::createEditor() { return new RaymondEditor(*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new RaymondProcessor(); }
