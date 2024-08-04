#include <nfd.h>
#include <string.h>
#include "core/editor.h"
#include "cimgui.h"
#include "core/asset_manager.h"
#include "core/renderer.h"
#include "core/components.h"
#include "core/ecs.h"
#include "util/log.h"
#include "global.h"

const u32 BASE_SIZE = 16;
const u32 SCALE     = 2;
const u32 TILE_SIZE = BASE_SIZE * SCALE;
static ecs_world_t *_world = NULL;

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

        igText("TILESETS");
        igSeparator();
        if (global.asset_manager->tilesets->count > 0) {
            array_list *tilesets = asset_manager_get_all_tileset(global.asset_manager);
            for (u32 i = 0; i < tilesets->len; i++) {
                entry_t *item = tilesets->data + i * tilesets->data_size;
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
                    asset_manager_push_tileset(global.asset_manager, out_path, 16);
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
    array_list *tilesets = asset_manager_get_all_tileset(global.asset_manager);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_AlwaysAutoResize;
    if (igBegin("Tile Editor", &global.editor_state.visible_tile_editor, flags)) {

        ImDrawList *drawlist = igGetWindowDrawList();
        tileset_t *tileset   = asset_manager_get_tileset(global.asset_manager, global.editor_state.tile_editor_state.selected_tileset);

        if (tileset) {
            u8          scale      = SCALE;
            ImTextureID texture_id = (void*)(size_t)tileset->texture.handle;
            ImVec2      image_size = {tileset->texture.size.x * scale, tileset->texture.size.y * scale};
            u32         rows       = image_size.y / TILE_SIZE;
            u32         cols       = image_size.x / TILE_SIZE;
            u32         tile_count = tileset->tile_count;


            // set to global state
            global.editor_state.tile_editor_state.texture_id = texture_id;
            global.editor_state.tile_editor_state.image_size = image_size;
            global.editor_state.tile_editor_state.tile_count = tile_count;
            global.editor_state.tile_editor_state.rows       = rows;
            global.editor_state.tile_editor_state.cols       = cols;

            // Get windowSize and windowPosition
            ImVec2 window_size, window_pos;
            igGetWindowSize(&window_size);
            igGetWindowPos(&window_pos);

            // calculate tilesetPosition
            ImVec2 tileset_margin = {8, 28};
            ImVec2 tileset_pos    = {window_pos.x + tileset_margin.x, window_pos.y + tileset_margin.y};


            // draw tileset image
            ImVec2 uv0 = {0.0f, 1.0f};
            ImVec2 uv1 = {1.0f, 0.0f};
            igImage(texture_id, image_size, uv0, uv1, (ImVec4){1.0f,1.0f,1.0f,1.0f}, (ImVec4){1.0f,1.0f,1.0f,1.0f});

            // draw grids
            { 
                ImVec4 gridColor  = {0.67, 0.67, 0.67, 1.0};
                ImU32 uGridColor  = igColorConvertFloat4ToU32(gridColor);
                f32 gridThickness = 1.0f;

                // draw vertical lines
                for (u32 y = 0; y < rows; y++) {
                    for (u32 x = 0; x < cols; x++) {
                        ImVec2 p1 = {tileset_pos.x + x * TILE_SIZE, tileset_pos.y + y * TILE_SIZE};
                        ImVec2 p2 = {tileset_pos.x + x * TILE_SIZE, tileset_pos.y + (y + 1) * TILE_SIZE};
                        ImDrawList_AddLine(drawlist, p1, p2, uGridColor, gridThickness);
                    }
                }

                // draw horizontal lines
                for (u32 x = 0; x < cols; x++) {
                    for (u32 y = 1; y < rows; y++) {
                        ImVec2 p1 = {tileset_pos.x + x * TILE_SIZE, tileset_pos.y + y * TILE_SIZE};
                        ImVec2 p2 = {tileset_pos.x + (x+1) * TILE_SIZE, tileset_pos.y + y * TILE_SIZE};
                        ImDrawList_AddLine(drawlist, p1, p2, uGridColor, gridThickness);
                    }
                }
            }

            // get local mouse position
            f32 mousex = global.window->mouse.xpos - window_pos.x + igGetScrollX();
            f32 mousey = global.window->mouse.ypos - window_pos.y + igGetScrollY();

            // convert mouse position to tileIndex
            ImVec2 tileset_idx = {floor((mousex - tileset_margin.x) / TILE_SIZE), floor((image_size.y - (mousey - tileset_margin.y)) / TILE_SIZE)};
            s32 selectTile     = -1;

            global.editor_state.tile_editor_state.mouse.x      = mousex;
            global.editor_state.tile_editor_state.mouse.y      = mousey;
            global.editor_state.tile_editor_state.mouse_rc.row = tileset_idx.y;
            global.editor_state.tile_editor_state.mouse_rc.col = tileset_idx.x;

            if (tileset_idx.x >= 0 && tileset_idx.y >= 0 && tileset_idx.x < cols && tileset_idx.y < rows) {
                selectTile = tileset_idx.x + (tileset_idx.y * cols);
                if (igGetMouseClickedCount(ImGuiMouseButton_Left)) { global.editor_state.tile_editor_state.selected_tile = selectTile; }
            }
        }

    }

    // change tileset from asset manager
    if (igBeginCombo("Change tileset", global.editor_state.tile_editor_state.selected_tileset, 0)) {
        if (tilesets != NULL) {
            for (u32 i = 0; i < tilesets->len; i++) {
                entry_t *item = (tilesets->data + i * tilesets->data_size);
                b8 is_select  = (global.editor_state.tile_editor_state.selected_tileset) 
                    ? (0 == strcmp(item->key, global.editor_state.tile_editor_state.selected_tileset)) 
                    : false;

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
        f32 sizeX = TILE_SIZE / global.editor_state.tile_editor_state.image_size.x;
        f32 sizeY = TILE_SIZE / global.editor_state.tile_editor_state.image_size.y;

        u32 cols = global.editor_state.tile_editor_state.cols;

        u32 idxX = global.editor_state.tile_editor_state.selected_tile % cols;
        u32 idxY = global.editor_state.tile_editor_state.selected_tile / cols;

        ImVec2 uv0  = {idxX * sizeX, idxY * sizeY + sizeY};
        ImVec2 uv1  = {idxX * sizeX + sizeX, idxY * sizeY};
        ImVec2 size = {TILE_SIZE, TILE_SIZE};
        igImage(global.editor_state.tile_editor_state.texture_id, size, uv0, uv1, (ImVec4){1.0,1.0,1.0,1.0}, (ImVec4){1.0,1.0,1.0,1.0}); 

        global.editor_state.tile_editor_state.selected_tile_idx = (vec2s){idxX,idxY};
        global.editor_state.tile_editor_state.tile_stride = (vec2s){sizeX,sizeY};

    }
    igEnd();
}

static void canvasDialog(void) {
    static ivec2s canvas_size = {0};
    static b8 apply          = false;

    if (igBegin((global.editor_state.canvas_state.created) ? "Edit canvas" : "Create new canvas", &global.editor_state.visible_canvas_dialog, ImGuiWindowFlags_NoCollapse)) {

        ImVec2 btn_size = {64,24};
        if (igInputInt("Canvas Width", &canvas_size.x, TILE_SIZE, TILE_SIZE, 0)) { 
            if (TILE_SIZE != 0 && canvas_size.x % TILE_SIZE != 0) {
                canvas_size.x -= (canvas_size.x % TILE_SIZE);
            }
        }
        if (igInputInt("Canvas Height", &canvas_size.y, TILE_SIZE, TILE_SIZE, 0)) { 
            if (TILE_SIZE != 0 && canvas_size.y % TILE_SIZE != 0) {
                canvas_size.y -= (canvas_size.y % TILE_SIZE); 
            }
        }

        if (igButton("Apply", btn_size) && TILE_SIZE != 0 && canvas_size.x != 0 && canvas_size.y != 0) {
            if (global.editor_state.canvas_state.created) {
                if(igBegin("Are you sure", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize)) {
                    if (igButton("Yes", btn_size)) {
                        renderer_clean(global.renderer);
                        ecs_killall(_world);

                        apply = true;
                    }
                    igSameLine(0.0f,12.0f);
                    if (igButton("No", btn_size)) {
                        global.editor_state.visible_canvas_dialog = false;

                        apply = false;
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
        global.editor_state.canvas_state.width      = canvas_size.x;
        global.editor_state.canvas_state.height     = canvas_size.y;
        global.editor_state.canvas_state.rows       = canvas_size.y / TILE_SIZE;
        global.editor_state.canvas_state.cols       = canvas_size.x / TILE_SIZE;
        global.editor_state.canvas_state.stride     = TILE_SIZE;
        global.editor_state.canvas_state.position.x = 0;
        global.editor_state.canvas_state.position.y = 0;
        global.editor_state.canvas_state.created    = true;

        for (s32 y = 0; y < global.editor_state.canvas_state.rows; y++) {
            for (s32 x = 0; x < global.editor_state.canvas_state.cols; x++) {
                ecs_entity_t entity_id = ecs_create(_world, (ecs_desc_t){3, {POSITION_COMPONENT, SPRITE_COMPONENT, UPDATABLE_COMPONENT} });

                f32 posX = x * global.editor_state.canvas_state.stride;
                f32 posY = y * global.editor_state.canvas_state.stride;
                ecs_set(_world, entity_id, POSITION_COMPONENT, &(Position){posX, posY, 0});
                ecs_set(_world, entity_id, SPRITE_COMPONENT, &(Sprite){0, TILE_SIZE, TILE_SIZE});
                ecs_set(_world, entity_id, UPDATABLE_COMPONENT, &(Updatable){0});
            }
        }


        ecs_query_t* query = ecs_query(_world);
        vec4s color = glms_vec4_zero();
        for (u32 i = 0; i < query->len; i++) {
            const Position *pos = ecs_get(_world,query->list[i], POSITION_COMPONENT);
            const Sprite   *spr = ecs_get(_world,query->list[i], SPRITE_COMPONENT);

            if (i % 2 == 0) {
                color = (vec4s){0.1f, 0.1f, 0.1f, 1.0};
            }
            else {
                color = (vec4s){0.2f, 0.2f, 0.2f, 1.0};
            }
            renderer_append_quad(global.renderer, (vec2s){spr->spriteWidth, spr->spriteHeight}, (vec3s){pos->x,pos->y,pos->z}, color);
        }

        apply = false;
        global.editor_state.visible_canvas_dialog = false;
    }
}

static void debug_info(void) {
    if (igBegin("Debug Info", &global.editor_state.visible_debug_info, ImGuiWindowFlags_NoCollapse)) {
        igText("ECS count (alive)  : %lu (%lu)", _world->entities->count, _world->entities->count_alive);
        igText("ECS component count: %u", _world->components->count);
        igSeparator();

        igText("Projection Size      : %.2f, %.2f", global.settings.projection_size.x, global.settings.projection_size.y);
        igText("Camera Position      : %.2f, %.2f, %.2f", global.camera->position.x, global.camera->position.y, global.camera->position.z);
        igText("Window Mouse Position: %.2f, %.2f", global.window->mouse.xpos, global.window->mouse.ypos);
        igText("Ortho Mouse Position : %.2f, %.2f", global.ortho_mouse.x, global.ortho_mouse.y);
        igSeparator();

        igText("Tile Mouse Position  : %.2f, %.2f", global.editor_state.tile_editor_state.mouse.x, global.editor_state.tile_editor_state.mouse.y);
        igText("Tile Mouse RC        : %d, %d", global.editor_state.tile_editor_state.mouse_rc.row, global.editor_state.tile_editor_state.mouse_rc.col);
        igText("Selected Tile(uv)    : %d", global.editor_state.tile_editor_state.selected_tile);
        igSeparator();

        if (global.editor_state.canvas_state.created) {
            igText("Canvas Tile Size     : %d", global.editor_state.canvas_state.stride);
            igText("Canvas Size          : %d, %d (%d, %d)", 
                    global.editor_state.canvas_state.width, global.editor_state.canvas_state.height,
                    global.editor_state.canvas_state.cols, global.editor_state.canvas_state.rows);
            igText("Canvas Position      : %.2f, %.2f", global.editor_state.canvas_state.position.x, global.editor_state.canvas_state.position.y);
            igText("Canvas Mouse Position: %.2f, %.2f", global.editor_state.canvas_state.mouse.x, global.editor_state.canvas_state.mouse.y);
            igText("Canvas Mouse RC      : %d, %d", global.editor_state.canvas_state.mouse_rc.row, global.editor_state.canvas_state.mouse_rc.col);

            if (global.editor_state.canvas_state.mouse_rc.row >= 0 
                    && global.editor_state.canvas_state.mouse_rc.col >= 0 
                    && global.editor_state.canvas_state.mouse_rc.row < global.editor_state.canvas_state.rows 
                    && global.editor_state.canvas_state.mouse_rc.col < global.editor_state.canvas_state.cols) {

                u32 row       = global.editor_state.canvas_state.mouse_rc.row;
                u32 col       = global.editor_state.canvas_state.mouse_rc.col;
                s32 entity_id = col + row * global.editor_state.canvas_state.cols;
                global.editor_state.canvas_state.point_entity_id = entity_id;

                if (entity_id >= 0) {
                    const Position *pos = ecs_get(_world, entity_id, POSITION_COMPONENT);
                    const Sprite   *spr = ecs_get(_world, entity_id, SPRITE_COMPONENT);

                    igText("Entity Id            : %lu", entity_id);
                    igText("Entity Pos           : %.2f, %.2f, %.2f", pos->x, pos->y, pos->z);
                    igText("Entity TextureId     : %u", spr->textureId);
                    igText("Entity Size          : %u, %u", spr->spriteWidth, spr->spriteHeight);

                    /* // temp */
                    if (global.tile_update) {
                        tileset_t *tileset = asset_manager_get_tileset(global.asset_manager, global.editor_state.tile_editor_state.selected_tileset);

                        renderer_push_texture(global.renderer, tileset->texture);
                        ecs_set(_world, entity_id, UPDATABLE_COMPONENT, &(Updatable){UPDATE_TEXTURE});
                    }
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
    assert(*editor != NULL);

    (*editor)->context = igCreateContext(NULL);
    (*editor)->io      = igGetIO();

    ImGui_ImplGlfw_InitForOpenGL(global.window->handle, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    igStyleColorsDark(NULL);

    global.editor_state.tile_editor_state.selected_tile    = -1;
    global.editor_state.tile_editor_state.selected_tileset = NULL;
    global.editor_state.canvas_state.created               = false;

    _world = global.world;
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
        global.editor_state.canvas_state.mouse.x      = global.ortho_mouse.x - global.editor_state.canvas_state.position.x;
        global.editor_state.canvas_state.mouse.y      = global.ortho_mouse.y - global.editor_state.canvas_state.position.y;
        global.editor_state.canvas_state.mouse_rc.col = (int)floor(global.editor_state.canvas_state.mouse.x / global.editor_state.canvas_state.stride);
        global.editor_state.canvas_state.mouse_rc.row = (int)floor(global.editor_state.canvas_state.mouse.y / global.editor_state.canvas_state.stride);
    }

    menubar();
    if (global.editor_state.visible_tile_editor)   tile_editor();
    if (global.editor_state.visible_canvas_dialog) canvasDialog();
    if (global.editor_state.visible_debug_info)    debug_info();
    if (global.editor_state.visible_asset_manager) asset_manager();
}

void editor_render(void) {
    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}
