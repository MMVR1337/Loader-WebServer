namespace CustomCheckbox {
    bool Checkbox(const char* label, bool* v) {
        ImGui::PushID(label);

        ImGui::Text(label);
        ImGui::SameLine();

        ImVec2 pos = ImGui::GetCursorScreenPos();
        float size = 12.f;
        ImRect bb(pos, ImVec2(pos.x + size, pos.y + size));
        ImGui::ItemSize(bb);
        if (!ImGui::ItemAdd(bb, ImGui::GetID(label))) {
            ImGui::PopID();
            return false;
        }

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, ImGui::GetID(label), &hovered, &held);
        if (pressed) *v = !*v;

        static float anim_t = 0.0f;
        anim_t = ImLerp(anim_t, *v ? 1.0f : 0.0f, ImGui::GetIO().DeltaTime * 12.0f);

        ImU32 bg_color = ImGui::GetColorU32(ImLerp(ImVec4(0.1f, 0.1f, 0.1f, 1.0f), ImVec4(0.2f, 0.2f, 0.2f, 1.0f), hovered ? 1.0f : 0.8f));
        ImU32 check_color = ImGui::GetColorU32(ImVec4(0.9f, 0.2f, 0.2f, anim_t));

        ImGui::GetWindowDrawList()->AddRectFilled(bb.Min, bb.Max, bg_color, 4.0f);
        if (anim_t > 0.0f) {
            ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(bb.Min.x + size * 0.5f, bb.Min.y + size * 0.5f), size * 0.3f * anim_t, check_color);
        }

        ImGui::PopID();

        return pressed;
    }
}
