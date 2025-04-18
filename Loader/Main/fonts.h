static const unsigned int NumOfSizes = 13;
float sizes[NumOfSizes] = {
    11.f, //0
    12.f, //1
    13.f, //2
    14.f, //3
    15.f, //4
    16.f, //5
    17.f, //6
    18.f, //7
    19.f, //8
    20.f, //9
    21.f, //10
    22.f, //11
    23.f  //12
};
ImFont* boldlabel;
ImFont* bold[NumOfSizes];
ImFont* regular_[NumOfSizes];
ImFont* medium[NumOfSizes];
ImFont* icon[NumOfSizes];
ImFont* iconico;

void InitFonts() {

    ImGuiStyle& style = ImGui::GetStyle();
    style.AntiAliasedLines = true;
    style.AntiAliasedFill = true;

    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig cfg;
    static struct rFonts {
        ImFont** fontname;
        unsigned char* bytes;
        int bytesize;
        int pixelsize;
        float size;
    };
    static auto InitFont = [&](ImFont** fontname, unsigned char* bytes, int bytesize, float size) -> void {
        *fontname = io.Fonts->AddFontFromMemoryTTF(bytes, bytesize, size, &cfg, io.Fonts->GetGlyphRangesCyrillic());
        };

    std::jthread a1([&] {
        static const unsigned regularsize = sizeof(regularchar);
        for (unsigned int i = 0; i < NumOfSizes; i++) {
            InitFont(&regular_[i], regularchar, regularsize, sizes[i]);
        }
        });
    std::jthread a2([&] {
        static const unsigned boldsize = sizeof(boldchar);
        for (unsigned int i = 0; i < NumOfSizes; i++) {
            InitFont(&bold[i], boldchar, boldsize, sizes[i]);
        }
        boldlabel = io.Fonts->AddFontFromMemoryTTF(boldchar, boldsize, 60, &cfg, io.Fonts->GetGlyphRangesCyrillic());
        });
    std::jthread a3([&] {
        static const unsigned mediumsize = sizeof(mediumchar);
        for (unsigned int i = 0; i < NumOfSizes; i++) {
            InitFont(&medium[i], mediumchar, mediumsize, sizes[i]);
        }
        });
    std::jthread a4([&] {
        static const unsigned iconsize = sizeof(iconchar);
        for (unsigned int i = 0; i < NumOfSizes; i++) {
            InitFont(&icon[i], iconchar, iconsize, sizes[i]);
        }
        iconico = io.Fonts->AddFontFromMemoryTTF(iconchar, iconsize, 35, &cfg, io.Fonts->GetGlyphRangesCyrillic());
        });

    a1.join();
    a2.join();
    a3.join();
    a4.join();
}