#include "UI.h"
#include "imgui.h"
#include "RSA.h"
#include "chat.h"
#include <iostream>

int state = 0; // 0 == disconnected , 1 == connecting , 2 == disconnected

//UI Stuff
ImVector<const char*> chat;
bool AutoScroll = true;
char InputBuf[1024];


int P = 512347543;
int Q = 512349029;

//used to add seive to make selecting P , Q auto
// but its really not worth it lmao
//#define PRIMES_LIMIT 20000
//std::vector<int> primes;

//some string operations
static int   Stricmp(const char* s1, const char* s2) { int d; while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; } return d; }
static int   Strnicmp(const char* s1, const char* s2, int n) { int d = 0; while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; n--; } return d; }
static char* Strdup(const char* s) { IM_ASSERT(s); size_t len = strlen(s) + 1; void* buf = malloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)s, len); }
static void  Strtrim(char* s) { char* str_end = s + strlen(s); while (str_end > s && str_end[-1] == ' ') str_end--; *str_end = 0; }

//add to the "log" (chat)
static void AddLog(const char* fmt, ...) IM_FMTARGS(2)
{
    // FIXME-OPT
    char buf[4024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
    buf[IM_ARRAYSIZE(buf) - 1] = 0;
    va_end(args);
    chat.push_back(Strdup(buf));
}

//int arr[PRIMES_LIMIT] = { 0 };

//current font
ImFont* fnt;

void UI::InitUI(){
    //sieve , not used anymore ...
    /*for (int i = 2; i < PRIMES_LIMIT; i++) {
        for (int j = i * i; j < PRIMES_LIMIT; j += i) {
            arr[j - 1] = 1;
        }
    }
    for (int i = 1; i < PRIMES_LIMIT; i++) {
        if (arr[i - 1] == 0)
            primes.push_back(i);
    }*/

    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig cfg;
    cfg.OversampleH = 1;
    cfg.OversampleV = 1;
    cfg.RasterizerMultiply = 1.5f;
    
    fnt = io.Fonts->AddFontFromFileTTF("res\\JetBrainsMono-Medium.ttf", 18.0f , &cfg);
}

static void log(const char* ptr) {
    AddLog("0%s", ptr);
}

static void message(const char* ptr) {
    AddLog("1%s", ptr);
}

void UI::DrawUI(bool* exit){
    ImGui::PushFont(fnt);

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Menu"))
		{
            if (state == 0) {
                if (ImGui::MenuItem("Start server")) {
                    start_chat(state, true, &log, &message, P, Q);
                }
                if (ImGui::MenuItem("Start client")) {
                    start_chat(state, false, &log, &message, P, Q);
                }
            }
            else {
                if (ImGui::MenuItem("Disconnect")) {
                    disconnect();
                }
            }

			if (ImGui::MenuItem("Exit")) {
                *exit = true;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}


	static int window_flags = 0;

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoDocking;
	window_flags |= ImGuiWindowFlags_NoBackground;

	static bool p_open = true;
	ImGui::Begin("Here we go ", &p_open, window_flags);
	ImGui::PopStyleVar(2);

	{ // widgets before the chat area
		ImGui::Text("Statues : ");
		ImGui::SameLine();
		switch (state) {
		case 0:
			ImGui::TextColored({ 1,0,0,1 }, "Disconnected");
			break;
		case 1:
			ImGui::TextColored({ 0,1,1,1 }, "Connecting..");
			break;
		case 2:
			ImGui::TextColored({ 0,1,0,1 }, "Connected");
			break;
		}
		
        if (state == 0) {
            ImGui::TextDisabled("Use the menu to connect ");

            ////this code is bad btw , (using bs should speed things up , but you really shouldn't be taking prime input from user lmao)
            //for (int i = 1; i < primes.size(); i++) {
            //    
            //    if (primes[i] > P && primes[i - 1] < P) {
            //        P = primes[i];
            //    }

            //    if (primes[i] > Q && primes[i - 1] < Q) {
            //        Q = primes[i];
            //    }

            //    if (primes[i] > P && primes[i] > Q) break;
            //}


            //if (P < primes[0])
            //    P = primes[0];

            //if (Q < primes[0])
            //    Q = primes[0];

            ImGui::InputInt("P", &P);
            ImGui::InputInt("Q", &Q);

            ImGui::SameLine(0, 100);

            ImGui::Text("N : %lld", P * Q);
        }
	}

	{ // chat area

        // Reserve enough left-over height for 1 separator + 1 input text
        ImGui::SeparatorText("Chat");

        const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar))
        {
            if (ImGui::BeginPopupContextWindow())
            {
                if (ImGui::Selectable("Clear")) {
                    chat.clear();
                }
                ImGui::EndPopup();
            }

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));
            for (int i = 0; i < chat.Size; i++)
            {
                const char* item = chat[i];
                
                ImVec4 color = {1,1,1,1};
                if (item[0] == '0') {
                    color = { 1,0,0,1 }; //log
                }

                if (item[0] == '1') {
                    color = { 1,0,1,1 }; //out message
                }

                if (item[0] == '2') {
                    color = { 1,1,1,1 }; //normal message
                }
                
                ImGui::PushStyleColor(ImGuiCol_Text, color);
                ImGui::TextUnformatted(item + 1);
                ImGui::PopStyleColor();
            }

            if ((AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
                ImGui::SetScrollHereY(1.0f);

            ImGui::PopStyleVar();
        }
        ImGui::EndChild();
        ImGui::Separator();
	}

	{ //input area
        bool reclaim_focus = false;
        ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll;
        if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), input_text_flags , nullptr , nullptr))
        {
            char* s = InputBuf;
            Strtrim(s);
            if (s[0]) {
                send(s);
                AddLog("2%s", s);
            }
            strcpy_s(InputBuf, "");


            reclaim_focus = true;
        }

        // Auto-focus on window apparition
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus)
            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
	}
	
    
	ImGui::End();

    ImGui::PopFont();

    //demo saves lifes
	//ImGui::ShowDemoWindow();
}

void UI::Terminate(){
    disconnect();
    waitForChat(); //wait until join so libStd don't fk us up
}

