#pragma once
// Order: hp, low, mid, midf, high, drive, thr, ratio, mix, depth, rvTime, bal, fb, div(beats), out, ceil
struct PresetDef { const char* name; float v[16]; };
static const char* const kIds[16] = { "hp","low","mid","midf","high","drive","thr","ratio","mix","depth","rvTime","bal","fb","div","out","ceil" };
static const PresetDef kPresets[] = {
 {"Bongo Flava",{30,3,-1.5f,400,3,.15f,-18,3,.36f,.7f,1.6f,.5f,.3f,.5f,1,-1}},
 {"Amapiano",{28,4.5f,-2,300,2,.2f,-20,3.5f,.36f,.8f,1.8f,.4f,.25f,.75f,1,-1}},
 {"Afrobeat",{30,3,0,1000,3.5f,.15f,-17,2.5f,.3f,.6f,1.3f,.5f,.3f,.5f,0,-1}},
 {"Singeli",{35,2,-1,500,4,.35f,-22,4,.25f,.5f,1,.4f,.2f,.25f,1.5f,-.8f}},
 {"Gengetone / Hip-Hop",{30,4,-1,350,2.5f,.25f,-19,3.5f,.3f,.5f,1.2f,.4f,.25f,.5f,1,-1}},
 {"Gospel",{40,1,1,2500,2,.05f,-16,2.5f,.45f,1,2.4f,.3f,.3f,.5f,0,-1}},
 {"Kwaya Choir",{90,0,-1.5f,300,3.5f,.05f,-18,2.5f,.4f,.9f,2.2f,.25f,.2f,.5f,0,-1}},
 {"Injili Praise",{40,2,.5f,2500,3,.12f,-17,3,.35f,.7f,1.5f,.4f,.25f,.5f,.5f,-1}},
 {"Injili Worship (Ibada)",{45,1,1,2200,2.5f,.04f,-16,2.2f,.5f,1,3.2f,.2f,.3f,.5f,0,-1}},
 {"Gospel Lead Vocal",{100,-1,1.5f,3000,3.5f,.08f,-20,3.5f,.35f,.8f,1.8f,.35f,.3f,.5f,0,-1}},
 {"Gospel Live Band",{35,2,0,1500,2.5f,.1f,-17,2.5f,.3f,.6f,1.6f,.4f,.25f,.5f,.5f,-1}},
 {"Cathedral Choir",{80,0,-1,400,2,.03f,-17,2,.55f,1,4.2f,.1f,.15f,.5f,0,-1}},
 {"Acapella Harmony",{85,.5f,-1,350,3,.03f,-18,2.5f,.45f,.9f,2.6f,.3f,.2f,.75f,0,-1}},
 {"Pop Wide",{35,2,.5f,3000,4,.1f,-18,3,.4f,.8f,1.8f,.5f,.35f,.5f,.5f,-1}},
 {"Vocal Bright",{90,-1,1.5f,3500,4,.1f,-20,3.5f,.4f,.8f,1.5f,.5f,.3f,.5f,0,-1}},
 {"Warm Tape",{30,3,-1,600,-1,.4f,-16,2.5f,.3f,.5f,1.2f,.7f,.4f,.5f,0,-1}},
 {"Master Clean",{25,1,0,1000,1.5f,.05f,-14,2,0,0,1,.5f,0,.5f,0,-.5f}},
 {"Master Loud",{28,2,-.5f,800,2.5f,.3f,-22,4,0,0,1,.5f,0,.5f,3,-.3f}} };
static const int kNumPresets = (int)(sizeof(kPresets) / sizeof(kPresets[0]));
