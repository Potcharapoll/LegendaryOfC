#include <stdlib.h>
#include <string.h>

#include "editor.h"
#include "cimgui.h"
#include "global.h"
#include "texture.h"
#include "log.h"
#include "ecs.h"
#include "components.h"

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

static void menubar(void) {
    if (igBeginMainMenuBar()) {
        if (igBeginMenu("World", true)){
            if (igMenuItem_Bool("Create New Canvas", "ctrl + shift + n", false, true)) { 
                global.editor_state.visible_canvas_create = true; 
            }
            igEndMenu();
        }
        if (global.editor_state.canvas_state.created) {
            if (igBeginMenu("Edit", true)){
                if (igMenuItem_Bool("Canvas Size", "ctrl + shift + n", false, true)) { global.editor_state.visible_canvas_create = true; }
                igEndMenu();
            }
        }
        if (igBeginMenu("View", true)){
            igCheckbox("Show Tile editor", &global.editor_state.visible_tile_editor);
            igCheckbox("Show Debug Info", &global.editor_state.visible_debug_info);
            igEndMenu();
        }
    }
    igEndMainMenuBar();
}

static void tile_editor(void) {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NoResize;
    if (igBegin("Tile Editor", &global.editor_state.visible_tile_editor, flags)) {

        struct Texture tileset = get_tileset(global.editor_state.tile_editor_state.selected_tileset);
        global.editor_state.tile_editor_state.tileSize = 32;
        global.editor_state.tile_editor_state.textureId = (void*)(size_t)tileset.handle;
        global.editor_state.tile_editor_state.imageSize = (ImVec2){tileset.size.x * 2, tileset.size.y * 2};
        global.editor_state.tile_editor_state.rowsCount = global.editor_state.tile_editor_state.imageSize.y / global.editor_state.tile_editor_state.tileSize;
        global.editor_state.tile_editor_state.colsCount = global.editor_state.tile_editor_state.imageSize.x / global.editor_state.tile_editor_state.tileSize;
        global.editor_state.tile_editor_state.tileCount = global.editor_state.tile_editor_state.rowsCount * global.editor_state.tile_editor_state.colsCount;

        // Get windowSize and windowPosition
        ImVec2 windowSize, windowPos;
        igGetWindowSize(&windowSize);
        igGetWindowPos(&windowPos);

        ImDrawList *drawlist = igGetWindowDrawList();

        // calculate tilesetPosition
        ImVec2 tilemapPos = {windowPos.x + 8, windowPos.y + 28};

        // draw tileset image
        igImage(global.editor_state.tile_editor_state.textureId, 
                global.editor_state.tile_editor_state.imageSize, 
                (ImVec2){0.0f,1.0f}, (ImVec2){1.0f, 0.0f}, (ImVec4){1.0f,1.0f,1.0f,1.0f}, (ImVec4){1.0f,1.0f,1.0f,1.0f});
        
        if (igBeginCombo("Change tileset", global.editor_state.tile_editor_state.selected_tileset, 0)) {
            for (int i = 0; i < 2; i++) {
                b8 is_select = (0 == strcmp(tileset_list[i], global.editor_state.tile_editor_state.selected_tileset));

                if (igSelectable_Bool(tileset_list[i], is_select, 0, (ImVec2){})) {
                    global.editor_state.tile_editor_state.selected_tileset = tileset_list[i];
                }

                if (is_select) igSetItemDefaultFocus();
            }
            igEndCombo();
        }

        // draw grids
        { 
            const ImVec4 gridColor = {0.67, 0.67, 0.67, 1.0};
            ImU32 uGridColor = igColorConvertFloat4ToU32(gridColor);
            f32 gridThickness = 1.0f;

            // draw vertical lines
            for (int y = 0; y < global.editor_state.tile_editor_state.rowsCount; y++) {
                for (int x = 0; x < global.editor_state.tile_editor_state.colsCount; x++) {
                    ImVec2 p1 = {tilemapPos.x + x * global.editor_state.tile_editor_state.tileSize, tilemapPos.y + y * global.editor_state.tile_editor_state.tileSize};
                    ImVec2 p2 = {tilemapPos.x + x * global.editor_state.tile_editor_state.tileSize, tilemapPos.y + (y + 1) * global.editor_state.tile_editor_state.tileSize};
                    ImDrawList_AddLine(drawlist, p1, p2, uGridColor, gridThickness);
                }
            }

            // draw horizontal lines
            for (int x = 0; x < global.editor_state.tile_editor_state.colsCount; x++) {
                for (int y = 1; y < global.editor_state.tile_editor_state.rowsCount; y++) {
                    ImVec2 p1 = {tilemapPos.x + x * global.editor_state.tile_editor_state.tileSize, tilemapPos.y + y * global.editor_state.tile_editor_state.tileSize};
                    ImVec2 p2 = {tilemapPos.x + (x+1) * global.editor_state.tile_editor_state.tileSize, tilemapPos.y + y * global.editor_state.tile_editor_state.tileSize};
                    ImDrawList_AddLine(drawlist, p1, p2, uGridColor, gridThickness);
                }
            }
        }

        // get local mouse position
        f32 mousex = global.window->mouse.xpos - windowPos.x + igGetScrollX();
        f32 mousey = global.window->mouse.ypos - windowPos.y + igGetScrollY();

        // convert mouse position to tileIndex
        ImVec2 tilemapIdx = {floor((mousex - 8) / global.editor_state.tile_editor_state.tileSize), floor((mousey - 28) / global.editor_state.tile_editor_state.tileSize)};
        int selectTile = -1;

        if (tilemapIdx.x >= 0 && tilemapIdx.y >= 0 
        && tilemapIdx.x < global.editor_state.tile_editor_state.colsCount 
        && tilemapIdx.y < global.editor_state.tile_editor_state.rowsCount) 
            selectTile = tilemapIdx.x + (tilemapIdx.y * global.editor_state.tile_editor_state.colsCount);

        // draw selected rect
        {
            const ImVec4 rectColor = {1.0, 1.0, 1.0, 1.0};
            ImU32 uRectColor = igColorConvertFloat4ToU32(rectColor);
            f32 rectThickness = 2.0f;
            if (selectTile >= 0 
            && selectTile < global.editor_state.tile_editor_state.tileCount 
            && igIsMouseHoveringRect(tilemapPos, 
                (ImVec2){ tilemapPos.x + global.editor_state.tile_editor_state.imageSize.x, tilemapPos.y + global.editor_state.tile_editor_state.imageSize.y }, 0)) {

                ImVec2 top_corner = {
                    tilemapPos.x+(tilemapIdx.x*global.editor_state.tile_editor_state.tileSize),
                    tilemapPos.y+(tilemapIdx.y*global.editor_state.tile_editor_state.tileSize)
                };
                ImVec2 bottom_corner = {top_corner.x + global.editor_state.tile_editor_state.tileSize, top_corner.y + global.editor_state.tile_editor_state.tileSize};
                ImDrawList_AddRect(drawlist, top_corner, bottom_corner, uRectColor, 0.0f, 0, rectThickness);
            }
        }

        if (igGetMouseClickedCount(ImGuiMouseButton_Left)) { global.editor_state.tile_editor_state.selected_tile = selectTile; }

        igText("Mouse (X, Y) : (%.2f, %.2f)", mousex, mousey);
        igText("TileIdx (R,C): (%.2f, %.2f)", tilemapIdx.x, tilemapIdx.y);
        igText("Selected tile: %d", selectTile);
    }
    igEnd();
}

static void createCanvas(void) {
    static int tileSize      = 0;
    static ivec2s canvasSize = {0,0};
    static b8 create_new     = false;
    static b8 apply          = false;

    if (igBegin("Create new canvas", &global.editor_state.visible_canvas_create, ImGuiWindowFlags_NoCollapse)) {
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

    if (apply && !create_new && global.editor_state.canvas_state.created) {
        if(igBegin("Are you sure", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize)) {
            if (igButton("Yes", (ImVec2){64,24})) {
                create_new = true;
                renderer_clean(global.renderer);
                ecs_killall();
            }
            igSameLine(0.0f,12.0f);
            if (igButton("No", (ImVec2){64,24})) {
                apply = false;
                create_new = false;
                global.editor_state.visible_canvas_create = false;
            }
        }
        igEnd();
        return;
    }

    // create new canvas if apply
    if (apply || create_new) {
        global.editor_state.canvas_state.width   = canvasSize.x;
        global.editor_state.canvas_state.height  = canvasSize.y;
        global.editor_state.canvas_state.rows    = canvasSize.y / tileSize;
        global.editor_state.canvas_state.cols    = canvasSize.x / tileSize;
        global.editor_state.canvas_state.size    = tileSize;
        global.editor_state.canvas_state.posx    = 0;
        global.editor_state.canvas_state.posy    = 0;
        global.editor_state.canvas_state.created = true;

        ecs_entity_t e;
        for (s32 y = 0; y < global.editor_state.canvas_state.rows; y++) {
            for (s32 x = 0; x < global.editor_state.canvas_state.cols; x++) {
                e = ecs_create();

                f32 posX = x * global.editor_state.canvas_state.size;
                f32 posY = y * global.editor_state.canvas_state.size;
                ecs_add(e.Id, POSITION_COMPONENT, &(positionComponent){posX, posY, 0});
                ecs_add(e.Id, SPRITE_COMPONENT, &(spriteComponent){0,0,0,tileSize, tileSize});
                ecs_add(e.Id, UPDATE_COMPONENT, &(updateComponent){0});
            }
        }

        ecs_query_t query = ecs_query(COMPONENT_LAST, POSITION_COMPONENT, SPRITE_COMPONENT, UPDATE_COMPONENT);
        f32 strip_color[] = {0.1f, 0.2f};

        for (u32 i = 0; i < query.len; i++) {
            positionComponent *pos = ecs_get(query.list[i], POSITION_COMPONENT);
            spriteComponent   *spr = ecs_get(query.list[i], SPRITE_COMPONENT);

            if ((i+1) % global.editor_state.canvas_state.cols == 0) {
                f32 swap = strip_color[1];
                strip_color[1] = strip_color[0];
                strip_color[0] = swap;
            }

            renderer_append_quad(global.renderer,
                    (vec2s){spr->spriteWidth, spr->spriteHeight}, 
                    (vec3s){pos->x,pos->y,pos->z}, 
                    (vec4s){strip_color[i%2],strip_color[i%2],strip_color[i%2],1.0});
        }

        apply = false;
        create_new = false;
        global.editor_state.visible_canvas_create = false;
    }
}

static void debug_info(void) {
    if (igBegin("Debug Info", &global.editor_state.visible_debug_info, ImGuiWindowFlags_NoCollapse)) {
        igText("Projection Size      : %.2f, %.2f", global.settings.projection_size.x, global.settings.projection_size.y);
        igText("Camera Position      : %.2f, %.2f, %.2f", global.camera->position.x, global.camera->position.y, global.camera->position.z);
        igText("Window Mouse Position: %.2f, %.2f", global.window->mouse.xpos, global.window->mouse.ypos);
        igText("Ortho Mouse Position : %.2f, %.2f", global.ortho_mouse.x, global.ortho_mouse.y);
        igSeparator();

        igText("Selected Tile(uv)    : %d", global.editor_state.tile_editor_state.selected_tile);
        igSeparator();

        if (global.editor_state.canvas_state.created) {
            igText("Canvas Tile Size     : %d", global.editor_state.canvas_state.size);
            igText("Canvas Size          : %d, %d (%d, %d)", 
                    global.editor_state.canvas_state.width, global.editor_state.canvas_state.height,
                    global.editor_state.canvas_state.cols, global.editor_state.canvas_state.rows);
            igText("Canvas Position      : %.2f, %.2f", global.editor_state.canvas_state.posx, global.editor_state.canvas_state.posy);
            igText("Canvas Mouse Position: %.2f, %.2f", global.editor_state.canvas_state.mousex, global.editor_state.canvas_state.mousey);
            igText("Canvas Mouse RC      : %d, %d", global.editor_state.canvas_state.mouse_row, global.editor_state.canvas_state.mouse_col);

            if (global.editor_state.canvas_state.mouse_row >= 0 
                    && global.editor_state.canvas_state.mouse_col >= 0 
                    && global.editor_state.canvas_state.mouse_row < global.editor_state.canvas_state.rows 
                    && global.editor_state.canvas_state.mouse_col < global.editor_state.canvas_state.cols) {

                u32 row = global.editor_state.canvas_state.mouse_row;
                u32 col = global.editor_state.canvas_state.mouse_col;
                s32 entity_id = col + row * global.editor_state.canvas_state.cols;

                if (entity_id >= 0) {
                    positionComponent *pos = ecs_get(entity_id, POSITION_COMPONENT);
                    spriteComponent   *spr = ecs_get(entity_id, SPRITE_COMPONENT);

                    igText("Entity Id            : %lu", entity_id);
                    igText("Entity Pos           : %.2f, %.2f, %.2f", pos->x, pos->y, pos->z);
                    igText("Entity TextureId     : %u", spr->textureId);
                    igText("Entity TextureSize   : %u, %u", spr->textureWidth, spr->textureHeight);
                    igText("Entity Size          : %u, %u", spr->spriteWidth, spr->spriteHeight);
                }
            } else {
                igText("Entity Id            :");
                igText("Entity Pos           :");
                igText("Entity TextureId     :");
                igText("Entity TextureSize   :");
                igText("Entity Size          :");
            }
        }
    }
    igEnd();
}

void editor_init(Editor **editor) {
    *editor = malloc(sizeof(**editor));
    (*editor)->context = igCreateContext(NULL);
    (*editor)->io = igGetIO();

    ImGui_ImplGlfw_InitForOpenGL(global.window->handle, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    igStyleColorsDark(NULL);

    // tileset
    tileset1 = texture_load(tileset_list[0]);
    tileset2 = texture_load(tileset_list[1]);
    
    global.editor_state.tile_editor_state.selected_tile    = -1;
    global.editor_state.tile_editor_state.selected_tileset = tileset_list[0];

    global.editor_state.canvas_state.created = false;
}

void editor_destroy(Editor *editor) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    igDestroyContext(editor->context);
    texture_destroy(tileset1);
    texture_destroy(tileset2);

    free(editor);
    LOG_DEBUG("Editor destroyed");
}

void editor_newframe(void) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();

    if (global.editor_state.canvas_state.created) {
        global.editor_state.canvas_state.mousex = global.ortho_mouse.x - global.editor_state.canvas_state.posx;
        global.editor_state.canvas_state.mousey = global.ortho_mouse.y - global.editor_state.canvas_state.posy;
        global.editor_state.canvas_state.mouse_col = (int)floor(global.editor_state.canvas_state.mousex / global.editor_state.canvas_state.size);
        global.editor_state.canvas_state.mouse_row = (int)floor(global.editor_state.canvas_state.mousey / global.editor_state.canvas_state.size);
    }

    menubar();
    if (global.editor_state.visible_tile_editor) tile_editor();
    if (global.editor_state.visible_canvas_create) createCanvas();
    if (global.editor_state.visible_debug_info) debug_info();
}

void editor_render(void) {
    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}
