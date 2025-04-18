void CustomInputText(const char* label, char* buffer, size_t buffer_size) {
    ImGui::PushID(label);

    ImVec2 inputSize = ImVec2(ImGui::CalcItemWidth(), ImGui::GetFrameHeight());

    ImGui::BeginGroup();
    ImGui::PushItemWidth(inputSize.x);

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_AlwaysOverwrite | ImGuiInputTextFlags_NoHorizontalScroll;
    ImGui::InputText("##hidden", buffer, buffer_size, flags);

    ImGui::PopItemWidth();

    if (strlen(buffer) == 0) {
        ImVec2 inputMin = ImGui::GetItemRectMin();
        ImVec2 inputMax = ImGui::GetItemRectMax();
        ImVec2 textSize = ImGui::CalcTextSize(label);

        ImVec2 textPos = ImVec2(
            inputMin.x + (inputSize.x - textSize.x) * 0.5f,
            inputMin.y + (inputSize.y - textSize.y) * 0.5f
        );

        ImGui::SetCursorScreenPos(textPos);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 0.5f));
        ImGui::TextUnformatted(label);
        ImGui::PopStyleColor();
    }

    ImGui::EndGroup();
    ImGui::PopID();
}
