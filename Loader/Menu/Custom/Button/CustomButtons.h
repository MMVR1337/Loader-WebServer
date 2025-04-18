// custombuttons.h
namespace CustomButtons {
    bool Button(const char* label, const ImVec2& size = ImVec2(0, 0)) {
        ImGui::PushID(label);

        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ImGui::ItemSize(bb);
        if (!ImGui::ItemAdd(bb, ImGui::GetID(label))) {
            ImGui::PopID();
            return false;
        }

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, ImGui::GetID(label), &hovered, &held);

        static float anim_t = 0.0f;
        anim_t = ImLerp(anim_t, hovered ? 1.0f : 0.0f, ImGui::GetIO().DeltaTime * 10.0f);

        ImU32 bg_color = ImGui::GetColorU32(ImLerp(ImVec4(0.2f, 0.2f, 0.2f, 1.0f), ImVec4(0.7f, 0.2f, 0.2f, 1.0f), anim_t));

        ImGui::GetWindowDrawList()->AddRectFilled(bb.Min, bb.Max, bg_color, 5.0f);

        ImGui::SetCursorScreenPos(ImVec2(pos.x + size.x * 0.25f, pos.y + size.y * 0.3f));

        ImGui::Text(label);

        ImGui::PopID();
        return pressed;
    }
}