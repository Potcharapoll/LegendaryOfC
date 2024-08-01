#include <stdlib.h>
#include <string.h>
#include <nfd.h>

#include "editor.h"
#include "global.h"
#include "cimgui.h"
#include "array_list.h"
#include "asset_manager.h"
#include "hashtable.h"
#include "renderer.h"
#include "texture.h"
#include "components.h"
#include "log.h"
#include "ecs.h"

static void asset_manager(void) {
    if (igBegin("Asset Manager", &global.editor_state.visible_asset_manager, ImGuiWindowFlags_NoCollapse)) {
        igText("SHADERS");
        igSeparator();
        if (global.asset_manager->shaders->count > 0) {
            array_list *shaders = asset_manager_get_all_shader(global.asset_manager);
            for (u32 i = 0; i < shaders->len; i++) {
                entry_t *item = shaders->data + i * shaders->data_size;
                igText("%s", item->key);
            }
        }

        igDummy((ImVec2){0.0f, 20.0f});

        igText("TEXTURES");
        igSeparator();
        if (global.asset_manager->textures->count > 0) {
            array_list *textures = asset_manager_get_all_texture(global.asset_manager);
            for (u32 i = 0; i < textures->len; i++) {
                entry_t *item = textures->data + i * textures->data_size;
                igText("%s", item->key);
            }
        }
    }
    igEnd();
}

static void menubar(void) {
    if (igBeginMainMenuBar()) {
        if (igBeginMenu("World", true)){
            if (igMenuItem_Bool("Create New Canvas", "ctrl + shift + n", false, true)) { global.editor_state.visible_canvas_dialog = true; }
            if (igMenuItem_Bool("Add tileset", "", false, true)) {

                NFD_Init();
                nfdu8char_t *out_path;
                nfdu8filteritem_t filter = { "Tileset", "png" };
                nfdopendialognargs_t args = {0};
                args.filterList  = &filter;
                args.filterCount = 1;
                nfdresult_t result = NFD_OpenDialogU8_With(&out_path, &args);
                if (result == NFD_OKAY) {
                    LOG_DEBUG("Load tileset from \'%s\'", out_path);
                    asset_manager_push_texture(global.asset_manager, out_path);
                }
                else {
                    LOG_ERROR("Failed to load tileset: %s", (result == NFD_CANCEL) ? "Cancel from user" : NFD_GetError());
                }
                NFD_Quit(); 
            }
            igEndMenu();
        }
        if (global.editor_state.canvas_state.created) {
            if (igBeginMenu("Edit", true)){
                if (igMenuItem_Bool("Canvas Size", "ctrl + shift + n", false, true)) { global.editor_state.visible_canvas_dialog = true; }
                igEndMenu();
            }
        }
        if (igBeginMenu("View", true)){
            igCheckbox("Show Tile editor", &global.editor_state.visible_tile_editor);
            igCheckbox("Show Debug Info", &global.editor_state.visible_debug_info);
            igCheckbox("Show Asset Manager", &global.editor_state.visible_asset_manager);
            igEndMenu();
        }
    }
    igEndMainMenuBar();
}

static void tile_editor(void) {
    array_list *textures = asset_manager_get_all_texture(global.asset_manager);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NoResize;
    if (igBegin("Tile Editor", &global.editor_state.visible_tile_editor, flags)) {

        ImDrawList *drawlist = igGetWindowDrawList();
        struct Texture* tileset = asset_manager_get_texture(global.asset_manager, global.editor_state.tile_editor_state.selected_tileset);

        if (tileset) {
            global.editor_state.tile_editor_state.tileSize  = (16 * 2);
            global.editor_state.tile_editor_state.textureId = (void*)(size_t)tileset->handle;
            global.editor_state.tile_editor_state.imageSize = (ImVec2){tileset->size.x * 2, tileset->size.y * 2}; // enlarge the tileset size 
            global.editor_state.tile_editor_state.rowsCount = global.editor_state.tile_editor_state.imageSize.y / global.editor_state.tile_editor_state.tileSize;
            global.editor_state.tile_editor_state.colsCount = global.editor_state.tile_editor_state.imageSize.x / global.editor_state.tile_editor_state.tileSize;
            global.editor_state.tile_editor_state.tileCount = global.editor_state.tile_editor_state.rowsCount * global.editor_state.tile_editor_state.colsCount;

            // Get windowSize and windowPosition
            ImVec2 windowSize, windowPos;
            igGetWindowSize(&windowSize);
            igGetWindowPos(&windowPos);

            // calculate tilesetPosition
            ImVec2 tilemapPos = {windowPos.x + 8, windowPos.y + 28};

            // draw tileset image
            igImage(global.editor_state.tile_editor_state.textureId, global.editor_state.tile_editor_state.imageSize,
                    (ImVec2){0.0f,1.0f}, (ImVec2){1.0f, 0.0f}, (ImVec4){1.0f,1.0f,1.0f,1.0f}, (ImVec4){1.0f,1.0f,1.0f,1.0f});

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
            ImVec2 tilemapIdx = {floor((mousex - 8) / global.editor_state.tile_editor_state.tileSize), 
                floor((global.editor_state.tile_editor_state.imageSize.y - (mousey - 28)) / global.editor_state.tile_editor_state.tileSize)};
            int selectTile = -1;

            global.editor_state.tile_editor_state.mousex    = mousex;
            global.editor_state.tile_editor_state.mousey    = mousey;
            global.editor_state.tile_editor_state.mouse_row = tilemapIdx.y;
            global.editor_state.tile_editor_state.mouse_col = tilemapIdx.x;

            if (tilemapIdx.x >= 0 && tilemapIdx.y >= 0 
                    && tilemapIdx.x < global.editor_state.tile_editor_state.colsCount 
                    && tilemapIdx.y < global.editor_state.tile_editor_state.rowsCount) {
                selectTile = tilemapIdx.x + (tilemapIdx.y * global.editor_state.tile_editor_state.colsCount);
                if (igGetMouseClickedCount(ImGuiMouseButton_Left)) { global.editor_state.tile_editor_state.selected_tile = selectTile; }
            }

            // FIXME: the rect doesn't draw at the correct position
            // draw selected rect
            {
                const ImVec4 rectColor = {1.0, 1.0, 1.0, 1.0};
                ImU32 uRectColor       = igColorConvertFloat4ToU32(rectColor);
                f32 rectThickness      = 2.0f;
                if (selectTile >= 0 
                        && selectTile < global.editor_state.tile_editor_state.tileCount 
                        && igIsMouseHoveringRect(tilemapPos, 
                            (ImVec2){ tilemapPos.x + global.editor_state.tile_editor_state.imageSize.x, tilemapPos.y + global.editor_state.tile_editor_state.imageSize.y }, 0)) {

                    ImVec2 top_corner = {
                        tilemapPos.x+(tilemapIdx.x * global.editor_state.tile_editor_state.tileSize),
                        tilemapPos.y+(tilemapIdx.y * global.editor_state.tile_editor_state.tileSize)
                    };
                    ImVec2 bottom_corner = {top_corner.x + global.editor_state.tile_editor_state.tileSize, top_corner.y + global.editor_state.tile_editor_state.tileSize};
                    ImDrawList_AddRect(drawlist, top_corner, bottom_corner, uRectColor, 0.0f, 0, rectThickness);
                }
            }

        }

    }

    // change tileset from asset manager
    if (igBeginCombo("Change tileset", global.editor_state.tile_editor_state.selected_tileset, 0)) {
        if (textures != NULL) {
            for (u32 i = 0; i < textures->len; i++) {
                entry_t *item = (textures->data + i * textures->data_size);

                b8 is_select  = (global.editor_state.tile_editor_state.selected_tileset) 
                    ? (0 == strcmp(item->key, global.editor_state.tile_editor_state.selected_tileset)) : false;

                if (igSelectable_Bool(item->key, is_select, 0, (ImVec2){})) {
                    global.editor_state.tile_editor_state.selected_tileset = item->key;
                }

                if (is_select) igSetItemDefaultFocus();
            }
        }
        igEndCombo();
    }

    // draw selected tile
    igText("Selected Tile: ");
    igSameLine(0.0f, 2.0f);
    if (global.editor_state.tile_editor_state.selected_tile >= 0) {
        f32 sizeX = global.editor_state.tile_editor_state.tileSize / global.editor_state.tile_editor_state.imageSize.x;
        f32 sizeY = global.editor_state.tile_editor_state.tileSize / global.editor_state.tile_editor_state.imageSize.y;

        u32 cols = global.editor_state.tile_editor_state.colsCount;

        u32 idxX = global.editor_state.tile_editor_state.selected_tile % cols;
        u32 idxY = global.editor_state.tile_editor_state.selected_tile / cols;

        ImVec2 uv0 = {idxX * sizeX, idxY * sizeY + sizeY};
        ImVec2 uv1 = {idxX * sizeX + sizeX, idxY * sizeY};
        igImage(global.editor_state.tile_editor_state.textureId, (ImVec2){32,32}, uv0, uv1, 
                (ImVec4){1.0,1.0,1.0,1.0}, (ImVec4){1.0,1.0,1.0,1.0});
    }

    igEnd();
}

static void canvasDialog(void) {
    static ivec2s canvasSize = {0};
    static b8 apply          = false;

    if (igBegin((global.editor_state.canvas_state.created) ? "Edit canvas" : "Create new canvas", &global.editor_state.visible_canvas_dialog, ImGuiWindowFlags_NoCollapse)) {
        if (igInputInt("Canvas Width", &canvasSize.x, 32, 32, 0)) { 
            if (32 != 0 && canvasSize.x % 32 != 0) {
                canvasSize.x -= (canvasSize.x % 32);
            }
        }
        if (igInputInt("Canvas Height", &canvasSize.y, 32, 32, 0)) { 
            if (32 != 0 && canvasSize.y % 32 != 0) {
                canvasSize.y -= (canvasSize.y % 32); 
            }
        }

        if (igButton("Apply", (ImVec2){64, 24}) && 32 != 0 && canvasSize.x != 0 && canvasSize.y != 0) {
            if (global.editor_state.canvas_state.created) {
                if(igBegin("Are you sure", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize)) {
                    if (igButton("Yes", (ImVec2){64,24})) {
                        apply = true;
                        renderer_clean(global.renderer);
                        ecs_killall();
                    }
                    igSameLine(0.0f,12.0f);
                    if (igButton("No", (ImVec2){64,24})) {
                        apply = false;
                        global.editor_state.visible_canvas_dialog = false;
                    }
                }
                igEnd();
            }
            else {
                apply = true;
            }
        }
    }
    igEnd();

    // create new canvas if apply
    if (apply) {
        global.editor_state.canvas_state.width   = canvasSize.x;
        global.editor_state.canvas_state.height  = canvasSize.y;
        global.editor_state.canvas_state.rows    = canvasSize.y / 32;
        global.editor_state.canvas_state.cols    = canvasSize.x / 32;
        global.editor_state.canvas_state.size    = 32;
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
                ecs_add(e.Id, SPRITE_COMPONENT, &(spriteComponent){0,0,0,32, 32});
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
        global.editor_state.visible_canvas_dialog = false;
    }
}

static void debug_info(void) {
    if (igBegin("Debug Info", &global.editor_state.visible_debug_info, ImGuiWindowFlags_NoCollapse)) {
        igText("Projection Size      : %.2f, %.2f", global.settings.projection_size.x, global.settings.projection_size.y);
        igText("Camera Position      : %.2f, %.2f, %.2f", global.camera->position.x, global.camera->position.y, global.camera->position.z);
        igText("Window Mouse Position: %.2f, %.2f", global.window->mouse.xpos, global.window->mouse.ypos);
        igText("Ortho Mouse Position : %.2f, %.2f", global.ortho_mouse.x, global.ortho_mouse.y);
        igSeparator();

        igText("Tile Mouse Position  : %.2f, %.2f", global.editor_state.tile_editor_state.mousex, global.editor_state.tile_editor_state.mousey);
        igText("Tile Mouse RC        : %d, %d", global.editor_state.tile_editor_state.mouse_row, global.editor_state.tile_editor_state.mouse_col);
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
                global.editor_state.canvas_state.pointed_entity_id = entity_id;

                if (entity_id >= 0) {
                    positionComponent *pos = ecs_get(entity_id, POSITION_COMPONENT);
                    spriteComponent   *spr = ecs_get(entity_id, SPRITE_COMPONENT);

                    igText("Entity Id            : %lu", entity_id);
                    igText("Entity Pos           : %.2f, %.2f, %.2f", pos->x, pos->y, pos->z);
                    igText("Entity TextureId     : %u", spr->textureId);
                    igText("Entity TextureSize   : %u, %u", spr->textureWidth, spr->textureHeight);
                    igText("Entity Size          : %u, %u", spr->spriteWidth, spr->spriteHeight);

                    /* // temp */
                    /* if (global.tile_update) { */
                    /*     spr->textureId = tileset1.handle; */
                    /*     spr->textureWidth = tileset1.size.x; */
                    /*     spr->textureHeight = tileset1.size.y; */

                    /*     renderer_push_texture(global.renderer, tileset1); */
                    /*     renderer_update_vertices(global.renderer, entity_id, selectedtileRow, selectedtileCol); */
                    /* } */
                }
            } 
            else {
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

    global.editor_state.tile_editor_state.selected_tile    = -1;
    global.editor_state.tile_editor_state.selected_tileset = NULL;

    global.editor_state.canvas_state.created = false;
}

void editor_destroy(Editor *editor) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    igDestroyContext(editor->context);

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
    if (global.editor_state.visible_canvas_dialog) canvasDialog();
    if (global.editor_state.visible_debug_info) debug_info();
    if (global.editor_state.visible_asset_manager) asset_manager();
}

void editor_render(void) {
    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}
