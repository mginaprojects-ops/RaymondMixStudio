#include "PluginEditor.h"

RaymondLNF::RaymondLNF()
{
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff120c22));
    setColour(juce::ComboBox::textColourId, juce::Colours::white);
    setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff3a2d5c));
    setColour(juce::ComboBox::arrowColourId, juce::Colour(0xffff3d9a));
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff120c22));
    setColour(juce::PopupMenu::textColourId, juce::Colours::white);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xffb13cff));
    setColour(juce::TextButton::buttonColourId, juce::Colour(0xff6a2fd0));
    setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    setColour(juce::ToggleButton::tickColourId, juce::Colour(0xffff3d9a));
    setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xff554a78));
    setColour(juce::Label::textColourId, juce::Colour(0xffffb13c));
    setColour(juce::Slider::thumbColourId, juce::Colour(0xff6a2fd0));
    setColour(juce::Slider::trackColourId, juce::Colour(0xffff3d9a));
    setColour(juce::Slider::backgroundColourId, juce::Colour(0xff2a1160));
}

void RaymondLNF::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s)
{
    auto b = juce::Rectangle<float>((float) x, (float) y, (float) w, (float) h).reduced(3.f);
    const float r = juce::jmin(b.getWidth(), b.getHeight()) / 2.f;
    const auto c = b.getCentre();

    juce::ColourGradient body(juce::Colour(0xff6a2fd0), c.x, c.y - r * .5f, juce::Colour(0xff0f0524), c.x, c.y + r, true);
    body.addColour(.6, juce::Colour(0xff2a1160));
    g.setGradientFill(body);
    g.fillEllipse(c.x - r, c.y - r, 2 * r, 2 * r);
    g.setColour(juce::Colours::black.withAlpha(.6f));
    g.drawEllipse(c.x - r, c.y - r, 2 * r, 2 * r, 1.5f);

    const float ar = r * .86f, th = juce::jmax(2.f, r * .08f);
    juce::Path track; track.addCentredArc(c.x, c.y, ar, ar, 0.f, a0, a1, true);
    g.setColour(juce::Colours::white.withAlpha(.15f));
    g.strokePath(track, juce::PathStrokeType(th, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const float ang = a0 + pos * (a1 - a0);
    juce::Path val; val.addCentredArc(c.x, c.y, ar, ar, 0.f, a0, juce::jmax(a0 + .001f, ang), true);
    juce::ColourGradient arc(juce::Colour(0xff7a3cff), c.x - r, c.y + r, juce::Colour(0xffffb13c), c.x + r, c.y - r, false);
    arc.addColour(.5, juce::Colour(0xffff3d9a));
    g.setGradientFill(arc);
    g.strokePath(val, juce::PathStrokeType(th * 1.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    g.setColour(juce::Colours::white);
    g.setFont(juce::jmax(10.f, r * .34f));
    g.drawText(s.getTextFromValue(s.getValue()), b.toNearestInt(), juce::Justification::centred);
}

RaymondEditor::RaymondEditor(RaymondProcessor& p) : AudioProcessorEditor(&p), proc(p)
{
    setLookAndFeel(&lnf);
    setSize(1000, 650);

    auto pct = [](double v) { return juce::String(juce::roundToInt(v * 100)) + "%"; };
    auto db1 = [](double v) { return (v > 0 ? "+" : "") + juce::String(v, 1); };
    auto i0 = [](double v) { return juce::String(juce::roundToInt(v)); };
    auto rate = [](double v)
    { static const char* n[5] = { "1/16", "1/8", "1/8.", "1/4", "1/2" }; return juce::String(n[juce::jlimit(0, 4, juce::roundToInt(v))]); };

    addKnob("rvTime", "REVERB TIME", { 30, 330, 140, 140 }, [](double v) { return juce::String(v, 2) + "s"; });
    addKnob("pre", "PRE ms", { 40, 500, 56, 56 }, i0);
    addKnob("color", "COLOR", { 110, 500, 56, 56 }, [](double v) { return juce::String(v / 1000.0, 1) + "k"; });
    addKnob("depth", "DEPTH", { 200, 372, 100, 100 }, pct);
    const char* eq[6][2] = { { "low", "LOW" }, { "mid", "MID" }, { "high", "HI" }, { "drive", "DRIVE" }, { "thr", "COMP" }, { "ratio", "RATIO" } };
    for (int i = 0; i < 6; ++i)
    {
        const juce::String id(eq[i][0]);
        std::function<juce::String(double)> f = (id == "drive") ? std::function<juce::String(double)>([](double v) { return juce::String(juce::roundToInt(v * 100)); })
                                                : (id == "ratio") ? std::function<juce::String(double)>([](double v) { return juce::String(v, 1); })
                                                : (id == "thr") ? std::function<juce::String(double)>([](double v) { return juce::String(juce::roundToInt(v)); })
                                                                : std::function<juce::String(double)>([](double v) { return juce::String(v, 1); });
        addKnob(id, eq[i][1], { 336 + i * 56, 345, 46, 46 }, f);
    }
    addKnob("mix", "DRY  MIX  WET", { 455, 432, 90, 90 }, pct);
    addKnob("fb", "FEEDBACK", { 700, 372, 100, 100 }, pct);
    addKnob("div", "DELAY RATE", { 830, 330, 140, 140 }, rate);
    addKnob("dlLp", "DELAY TONE", { 870, 500, 56, 56 }, [](double v) { return juce::String(v / 1000.0, 1) + "k"; });
    addKnob("inp", "INPUT dB", { 24, 584, 46, 46 }, db1);
    addKnob("bpm", "BPM", { 470, 584, 46, 46 }, i0);
    addKnob("ceil", "CEILING", { 880, 584, 46, 46 }, db1);
    addKnob("out", "OUTPUT dB", { 934, 584, 46, 46 }, db1);

    for (int i = 0; i < kNumPresets; ++i) preset.addItem(proc.getProgramName(i), i + 1);
    preset.setSelectedItemIndex(proc.getCurrentProgram(), juce::dontSendNotification);
    preset.onChange = [this] { proc.setCurrentProgram(preset.getSelectedItemIndex()); };
    preset.setBounds(350, 12, 230, 32); addAndMakeVisible(preset);

    if (auto* c = dynamic_cast<juce::AudioParameterChoice*>(proc.apvts.getParameter("rvType"))) rvType.addItemList(c->choices, 1);
    if (auto* c = dynamic_cast<juce::AudioParameterChoice*>(proc.apvts.getParameter("dlType"))) dlType.addItemList(c->choices, 1);
    rvA = std::make_unique<CA>(proc.apvts, "rvType", rvType);
    dlA = std::make_unique<CA>(proc.apvts, "dlType", dlType);
    rvType.setBounds(60, 140, 160, 28); dlType.setBounds(780, 140, 160, 28);
    addAndMakeVisible(rvType); addAndMakeVisible(dlType);

    rvOnA = std::make_unique<BA>(proc.apvts, "rvOn", rvOn);
    dlOnA = std::make_unique<BA>(proc.apvts, "dlOn", dlOn);
    byA = std::make_unique<BA>(proc.apvts, "bypass", bypass);
    atA = std::make_unique<BA>(proc.apvts, "autoTempo", autoT);
    rvOn.setBounds(60, 104, 160, 28); dlOn.setBounds(780, 104, 160, 28);
    bypass.setBounds(850, 14, 130, 28); autoT.setBounds(720, 52, 160, 26);
    for (auto* b : { &rvOn, &dlOn, &bypass, &autoT }) addAndMakeVisible(b);

    bal.setSliderStyle(juce::Slider::LinearHorizontal);
    bal.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    balA = std::make_unique<SA>(proc.apvts, "bal", bal);
    bal.setBounds(330, 526, 340, 22); addAndMakeVisible(bal);

    loadBtn.setBounds(595, 12, 110, 32); exportBtn.setBounds(712, 12, 120, 32);
    addAndMakeVisible(loadBtn); addAndMakeVisible(exportBtn);
    tempo.setBounds(24, 52, 680, 24); status.setBounds(24, 76, 940, 20);
    tempo.setFont(juce::Font(juce::FontOptions(15.f))); status.setFont(juce::Font(juce::FontOptions(13.f)));
    addAndMakeVisible(tempo); addAndMakeVisible(status);
    status.setText("Bonyeza \"Load Audio\" kupakia audio uliyoexport kutoka Cubase / Studio One.", juce::dontSendNotification);

    loadBtn.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser>("Chagua audio", juce::File {}, "*.wav;*.mp3;*.flac;*.aiff;*.aif;*.ogg;*.m4a");
        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto f = fc.getResult();
                if (f == juce::File {}) return;
                const double b = proc.loadAudio(f);
                status.setText(b > 0 ? "Imepakiwa: " + f.getFileName() + "  |  Tempo imetambuliwa: " + juce::String(b, 1) + " BPM  |  Delay imesync"
                                     : "Imeshindwa kusoma faili hii (au ni fupi sana kutambua tempo).", juce::dontSendNotification);
            });
    };
    exportBtn.onClick = [this]
    {
        if (!proc.hasAudio()) { status.setText("Pakia audio kwanza (Load Audio).", juce::dontSendNotification); return; }
        chooser = std::make_unique<juce::FileChooser>("Hifadhi mix kama WAV", juce::File::getSpecialLocation(juce::File::userMusicDirectory).getChildFile("raymond-mix.wav"), "*.wav");
        chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting,
            [this](const juce::FileChooser& fc)
            {
                auto f = fc.getResult();
                if (f == juce::File {}) return;
                status.setText("Inatengeneza mix …", juce::dontSendNotification);
                const bool ok = proc.renderToWav(f.withFileExtension("wav"));
                status.setText(ok ? "Imekamilika: " + f.withFileExtension("wav").getFullPathName() : "Imeshindwa kuhifadhi faili.", juce::dontSendNotification);
            });
    };
    startTimerHz(30);
}

RaymondEditor::~RaymondEditor() { stopTimer(); setLookAndFeel(nullptr); }

void RaymondEditor::addKnob(const juce::String& id, const juce::String& label, juce::Rectangle<int> r, std::function<juce::String(double)> fmt)
{
    auto k = std::make_unique<Knob>();
    k->label = label; k->r = r;
    k->s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    k->s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    k->s.setRotaryParameters(juce::degreesToRadians(225.f), juce::degreesToRadians(495.f), true);
    k->s.textFromValueFunction = fmt;
    k->att = std::make_unique<SA>(proc.apvts, id, k->s);
    k->s.setBounds(r);
    addAndMakeVisible(k->s);
    knobs.push_back(std::move(k));
}

void RaymondEditor::timerCallback()
{
    orb += (proc.outLevel.load() - orb) * .25f;
    inL += (proc.inLevel.load() - inL) * .3f;
    outL += (proc.outLevel.load() - outL) * .3f;
    static const float beats[5] = { .25f, .5f, .75f, 1.f, 2.f };
    const double b = proc.currentBpm.load();
    const int idx = juce::jlimit(0, 4, (int) proc.apvts.getRawParameterValue("div")->load());
    tempo.setText("TEMPO " + juce::String(b, 1) + " BPM [" + (proc.bpmSource.load() ? "DAW HOST" : "FILE / MANUAL")
                      + "]   |   DELAY = " + juce::String(juce::roundToInt(60000.0 / b * beats[idx])) + " ms",
                  juce::dontSendNotification);
    repaint();
}

void RaymondEditor::paint(juce::Graphics& g)
{
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff2a0f4d), 150.f, 0.f, juce::Colour(0xff05030c), 500.f, 300.f, true));
    g.fillAll();
    g.setColour(juce::Colour(0xffff3d9a).withAlpha(.12f)); g.fillEllipse(650.f, -150.f, 500.f, 400.f);

    g.setFont(22.f); g.setColour(juce::Colours::white);
    g.drawText("RAYMOND", 24, 10, 130, 36, juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xffff3d9a)); g.drawText("MIX", 140, 10, 60, 36, juce::Justification::centredLeft);
    g.setColour(juce::Colours::white); g.drawText("STUDIO", 190, 10, 130, 36, juce::Justification::centredLeft);

    // icons
    g.setColour(juce::Colour(0xffa24bff)); g.drawRoundedRectangle(108.f, 184.f, 64.f, 64.f, 16.f, 2.5f);
    g.setColour(juce::Colour(0xffa24bff).withAlpha(.2f)); g.fillRoundedRectangle(108.f, 184.f, 64.f, 64.f, 16.f);
    g.setColour(juce::Colour(0xffff3d6a)); g.drawRoundedRectangle(828.f, 184.f, 64.f, 64.f, 16.f, 2.5f);
    g.drawEllipse(839.f, 202.f, 28.f, 28.f, 2.5f); g.drawEllipse(853.f, 202.f, 28.f, 28.f, 2.5f);
    g.setColour(juce::Colour(0xffa99bc8)); g.setFont(14.f);
    g.drawText("REVERB", 60, 256, 160, 20, juce::Justification::centred);
    g.drawText("DELAY", 780, 256, 160, 20, juce::Justification::centred);

    // orb + orbits
    const juce::Point<float> c(500.f, 205.f);
    const float t = (float)(juce::Time::getMillisecondCounterHiRes() / 1000.0);
    juce::ColourGradient gr(juce::Colour(0xffff3d9a), c.x, c.y, juce::Colour(0x00783cff), c.x + 80.f + orb * 90.f, c.y, true);
    gr.addColour(.55, juce::Colour(0xffb13cff));
    g.setGradientFill(gr);
    const float rr = 62.f + orb * 50.f;
    g.fillEllipse(c.x - rr, c.y - rr, rr * 2, rr * 2);
    const juce::Colour oc[4] = { juce::Colour(0x73be78ff), juce::Colour(0x66ff5aaa), juce::Colour(0x4dffb13c), juce::Colour(0x4d5ac8ff) };
    for (int k = 0; k < 4; ++k)
    {
        juce::Path e; e.addEllipse(c.x - (190.f - k * 12.f), c.y - (48.f + k * 14.f), 2 * (190.f - k * 12.f), 2 * (48.f + k * 14.f));
        e.applyTransform(juce::AffineTransform::rotation(k * .7f + t * .12f * (k % 2 ? 1.f : -1.f), c.x, c.y));
        g.setColour(oc[k]); g.strokePath(e, juce::PathStrokeType(1.2f));
    }

    // silver panel
    juce::ColourGradient sv(juce::Colour(0xfff0eff5), 0.f, 310.f, juce::Colour(0xffb9b6c6), 0.f, 556.f, false);
    g.setGradientFill(sv); g.fillRoundedRectangle(10.f, 310.f, 980.f, 246.f, 38.f);
    g.setColour(juce::Colour(0xff0a0614)); g.fillRoundedRectangle(320.f, 330.f, 360.f, 96.f, 12.f);
    g.setColour(juce::Colours::white); g.drawRoundedRectangle(320.f, 330.f, 360.f, 96.f, 12.f, 2.f);
    g.setColour(juce::Colour(0xff5a5070)); g.setFont(12.f);
    g.drawText("ALGO CONTROL", 30, 478, 150, 16, juce::Justification::centred);
    g.drawText("REVERB", 290, 504, 60, 16, juce::Justification::centred);
    g.drawText("DELAY", 650, 504, 60, 16, juce::Justification::centred);

    // bottom bar
    g.setColour(juce::Colour(0xff0a0614)); g.fillRect(0, 556, 1000, 94);
    auto meter = [&](float x, float w, float lv)
    {
        g.setColour(juce::Colour(0xff1d1432)); g.fillRoundedRectangle(x, 604.f, w, 8.f, 4.f);
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff33ccff), x, 0.f, juce::Colour(0xffff3d9a), x + w, 0.f, false));
        g.fillRoundedRectangle(x, 604.f, w * juce::jlimit(0.f, 1.f, lv), 8.f, 4.f);
    };
    meter(90.f, 360.f, inL); meter(560.f, 290.f, outL);
    g.setColour(juce::Colour(0xff8d84a8)); g.setFont(11.f);
    g.drawText("INPUT", 90, 586, 60, 14, juce::Justification::left);
    g.drawText("OUTPUT", 560, 586, 60, 14, juce::Justification::left);

    for (auto& k : knobs)
    {
        g.setColour(k->r.getY() >= 556 ? juce::Colour(0xff8d84a8) : juce::Colour(0xff5a5070));
        g.setFont(11.f);
        g.drawText(k->label, k->r.getX() - 25, k->r.getBottom(), k->r.getWidth() + 50, 16, juce::Justification::centred);
    }
}
