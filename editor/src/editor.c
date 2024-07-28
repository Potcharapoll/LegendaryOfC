#include <stdlib.h>
#include <string.h>

#include "editor.h"
#include "cimgui.h"
#include "global.h"
#include "texture.h"

typedef struct {
    char *selected_tileset;
    ImTextureID textureId;

    ImVec2 imageSize;
    s32 rowsCount, colsCount;
    s32 tileCount, tileSize;
}TileEditorState;

static Editor *editor = NULL;
static TileEditorState tileEditorState;
static b8 showTileEditor = true;
static b8 create_canvas = false;
static struct Texture tileset1, tileset2;

// we could have a linked list to store list of tileset that has pushed
// and change it using tile editor
static char *tileset_list[] = {
    "../res/images/global.png",
    "../res/images/decorationsAndBlocks.png"
};

static struct Texture get_tileset(char *tileset) {
    return (0 == strcmp(tileset_list[0], tileset)) ? tileset1 : tileset2;
}

static inline void menubar(void) {
    if (igBeginMainMenuBar()) {
        if (igBeginMenu("World", true)){
            if (igMenuItem_Bool("Create new canvas", "ctrl + shift + n", false, true)) { create_canvas = true; }
            /* if (igMenuItem_Bool("Add Tileset", "", false, true)) { DEBUG_EVENT("Add tileset"); } */
            igEndMenu();
        }
        if (igBeginMenu("View", true)){
            igCheckbox("Show Tile editor", &showTileEditor);
            igEndMenu();
        }
    }
    igEndMainMenuBar();
}

static inline void tile_editor(void) {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NoResize;
    if (igBegin("Tile Editor", &showTileEditor, flags)) {

        struct Texture tileset = get_tileset(tileEditorState.selected_tileset);
        tileEditorState.tileSize = 32;
        tileEditorState.textureId = (void*)(size_t)tileset.handle;
        tileEditorState.imageSize = (ImVec2){tileset.size.x * 2, tileset.size.y * 2};
        tileEditorState.rowsCount = tileEditorState.imageSize.y / tileEditorState.tileSize;
        tileEditorState.colsCount = tileEditorState.imageSize.x / tileEditorState.tileSize;
        tileEditorState.tileCount = tileEditorState.rowsCount * tileEditorState.colsCount;

        // Get windowSize and windowPosition
        ImVec2 windowSize, windowPos;
        igGetWindowSize(&windowSize);
        igGetWindowPos(&windowPos);

        ImDrawList *drawlist = igGetWindowDrawList();

        // calculate tilesetPosition
        ImVec2 tilemapPos = {windowPos.x + 8, windowPos.y + 28};

        // draw tileset image
        igImage(tileEditorState.textureId, tileEditorState.imageSize, (ImVec2){0.0f,1.0f}, (ImVec2){1.0f, 0.0f}, (ImVec4){1.0f,1.0f,1.0f,1.0f}, (ImVec4){1.0f,1.0f,1.0f,1.0f});
        
        if (igBeginCombo("Change tileset", tileEditorState.selected_tileset, 0)) {
            for (int i = 0; i < 2; i++) {
                b8 is_select = (0 == strcmp(tileset_list[i], tileEditorState.selected_tileset));

                if (igSelectable_Bool(tileset_list[i], is_select, 0, (ImVec2){})) {
                    tileEditorState.selected_tileset = tileset_list[i];
                    /* DEBUG_EVENT("Tileset has changed"); */
                }

                if (is_select)
                    igSetItemDefaultFocus();
            }
            igEndCombo();
        }

        // draw grids
        { 
            const ImVec4 gridColor = {0.67, 0.67, 0.67, 1.0};
            ImU32 uGridColor = igColorConvertFloat4ToU32(gridColor);
            f32 gridThickness = 1.0f;

            // draw vertical lines
            for (int y = 0; y < tileEditorState.rowsCount; y++) {
                for (int x = 0; x < tileEditorState.colsCount; x++) {
                    ImVec2 p1 = {tilemapPos.x + x * tileEditorState.tileSize, tilemapPos.y + y * tileEditorState.tileSize};
                    ImVec2 p2 = {tilemapPos.x + x * tileEditorState.tileSize, tilemapPos.y + (y + 1) * tileEditorState.tileSize};
                    ImDrawList_AddLine(drawlist, p1, p2, uGridColor, gridThickness);
                }
            }

            // draw horizontal lines
            for (int x = 0; x < tileEditorState.colsCount; x++) {
                for (int y = 1; y < tileEditorState.rowsCount; y++) {
                    ImVec2 p1 = {tilemapPos.x + x * tileEditorState.tileSize, tilemapPos.y + y * tileEditorState.tileSize};
                    ImVec2 p2 = {tilemapPos.x + (x+1) * tileEditorState.tileSize, tilemapPos.y + y * tileEditorState.tileSize};
                    ImDrawList_AddLine(drawlist, p1, p2, uGridColor, gridThickness);
                }
            }
        }

        // get local mouse position
        f32 mousex = global.window->mouse.xpos - windowPos.x + igGetScrollX();
        f32 mousey = global.window->mouse.ypos - windowPos.y + igGetScrollY();

        // convert mouse position to tileIndex
        ImVec2 tilemapIdx = {floor((mousex - 8) / tileEditorState.tileSize), floor((mousey - 28) / tileEditorState.tileSize)};
        int selectTile = -1;

        if (tilemapIdx.x >= 0 && tilemapIdx.y >= 0 && tilemapIdx.x < tileEditorState.colsCount && tilemapIdx.y < tileEditorState.rowsCount) 
            selectTile = tilemapIdx.x + (tilemapIdx.y * tileEditorState.colsCount);

        // draw selected rect
        {
            const ImVec4 rectColor = {1.0, 1.0, 1.0, 1.0};
            ImU32 uRectColor = igColorConvertFloat4ToU32(rectColor);
            f32 rectThickness = 2.0f;
            if (selectTile >= 0 && selectTile < tileEditorState.tileCount && 
                    igIsMouseHoveringRect(tilemapPos,(ImVec2){tilemapPos.x + tileEditorState.imageSize.x, tilemapPos.y + tileEditorState.imageSize.y}, 0)) {

                ImVec2 top_corner = {tilemapPos.x+(tilemapIdx.x*tileEditorState.tileSize),tilemapPos.y+(tilemapIdx.y*tileEditorState.tileSize)};
                ImVec2 bottom_corner = {top_corner.x + tileEditorState.tileSize, top_corner.y + tileEditorState.tileSize};
                ImDrawList_AddRect(drawlist, top_corner, bottom_corner, uRectColor, 0.0f, 0, rectThickness);
            }
        }

        /* if (igGetMouseClickedCount(ImGuiMouseButton_Left)) { selected_tile = selectTile; } */

        igText("Mouse (X, Y) : (%.2f, %.2f)", mousex, mousey);
        igText("TileIdx (R,C): (%.2f, %.2f)", tilemapIdx.x, tilemapIdx.y);
        igText("Selected tile: %d", selectTile);
    }
    igEnd();
}

static void createCanvas(void) {
    static bool apply = false;

    static int tileSize = 0;
    static ivec2s canvasSize = {0,0};

    if (igBegin("Create new canvas", &create_canvas, ImGuiWindowFlags_NoCollapse)) {
        if (igInputInt("Tile Size", &tileSize, 2, 2, 0)) { }
        if (igInputInt("Canvas Width", &canvasSize.x, tileSize, tileSize, 0)) { 
            if (tileSize != 0 && canvasSize.x % tileSize != 0) {
                canvasSize.x -= (canvasSize.x % tileSize);
            }
        }
        if (igInputInt("Canvas Height", &canvasSize.y, tileSize, tileSize, 0)) { 
            if (tileSize != 0 && canvasSize.y % tileSize != 0) {
                canvasSize.y -= (canvasSize.y % tileSize); 
            }
        }

        if (igSmallButton("Apply") && tileSize != 0 && canvasSize.x != 0 && canvasSize.y != 0) {
            apply = true;
        }
    }
    igEnd();

    if (apply) {
        // create new canvas here
        apply = false;
    }
}

void editor_init(void) {
    editor = malloc(sizeof(*editor));
    editor->context = igCreateContext(NULL);
    editor->io = igGetIO();

    editor->io->ConfigFlags |= ImGuiWindowFlags_Popup;

    ImGui_ImplGlfw_InitForOpenGL(global.window->handle, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    igStyleColorsDark(NULL);

    // tileset
    tileset1 = texture_load(tileset_list[0]);
    tileset2 = texture_load(tileset_list[1]);
    tileEditorState.selected_tileset = tileset_list[0];
}

void editor_destroy(void) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    igDestroyContext(editor->context);
    texture_destroy(tileset1);
    texture_destroy(tileset2);

    free(editor);
}

void editor_newframe(void) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();

    menubar();
    if (showTileEditor) tile_editor();
}

void editor_render(void) {
    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}

Editor *get_editor_instance(void) {
    return editor;
}
