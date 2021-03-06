/*
 * Copyright 2002 - Florian Schulze <crow@icculus.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * This file is part of vt.
 *
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_endian.h>
#include "l_main.h"
#include "../core/system.h"

#define RCSID "$Id: l_tr2.cpp,v 1.15 2002/09/20 15:59:02 crow Exp $"

void TR_Level::read_tr2_colour4(SDL_RWops * const src, tr2_colour_t & colour)
{
    // read 6 bit color and change to 8 bit
    colour.r = read_bitu8(src) << 2;
    colour.g = read_bitu8(src) << 2;
    colour.b = read_bitu8(src) << 2;
    colour.a = read_bitu8(src) << 2;
}

void TR_Level::read_tr2_palette16(SDL_RWops * const src, tr2_palette_t & palette)
{
    for (int i = 0; i < 256; i++)
        read_tr2_colour4(src, palette.colour[i]);
}

void TR_Level::read_tr2_textile16(SDL_RWops * const src, tr2_textile16_t & textile)
{
    for (int i = 0; i < 256; i++) {
        if (SDL_RWread(src, textile.pixels[i], 2, 256) < 256)
            Sys_extError("read_tr2_textile16");

        for (int j = 0; j < 256; j++)
            textile.pixels[i][j] = SDL_SwapLE16(textile.pixels[i][j]);
    }
}

void TR_Level::read_tr2_box(SDL_RWops * const src, tr_box_t & box)
{
    box.zmax =-1024 * read_bitu8(src);
    box.zmin =-1024 * read_bitu8(src);
    box.xmin = 1024 * read_bitu8(src);
    box.xmax = 1024 * read_bitu8(src);
    box.true_floor =-read_bit16(src);
    box.overlap_index = read_bitu16(src);
}

void TR_Level::read_tr2_zone(SDL_RWops * const src, tr2_zone_t & zone)
{
    zone.GroundZone1_Normal = read_bit16(src);
    zone.GroundZone2_Normal = read_bit16(src);
    zone.GroundZone3_Normal = read_bit16(src);
    zone.GroundZone4_Normal = read_bit16(src);
    zone.FlyZone_Normal = read_bit16(src);
    zone.GroundZone1_Alternate = read_bit16(src);
    zone.GroundZone2_Alternate = read_bit16(src);
    zone.GroundZone3_Alternate = read_bit16(src);
    zone.GroundZone4_Alternate = read_bit16(src);
    zone.FlyZone_Alternate = read_bit16(src);
}

void TR_Level::read_tr2_room_light(SDL_RWops * const src, tr5_room_light_t & light)
{
    read_tr_vertex32(src, light.pos);
    light.intensity1 = read_bitu16(src);
    light.intensity2 = read_bitu16(src);
    light.fade1 = read_bitu32(src);
    light.fade2 = read_bitu32(src);

    light.intensity = light.intensity1;
    light.intensity /= 4096.0f;

    if(light.intensity > 1.0f)
        light.intensity = 1.0f;

    light.r_outer = light.fade1;
    light.r_inner = light.fade1 / 2;

    light.light_type = 0x01; // Point light

    // all white
    light.color.r = 0xff;
    light.color.g = 0xff;
    light.color.b = 0xff;
}

void TR_Level::read_tr2_room_vertex(SDL_RWops * const src, tr5_room_vertex_t & room_vertex)
{
    read_tr_vertex16(src, room_vertex.vertex);
    // read and make consistent
    room_vertex.lighting1 = (8191 - read_bit16(src)) << 2;
    room_vertex.attributes = read_bitu16(src);
    room_vertex.lighting2 = (8191 - read_bit16(src)) << 2;
    // only in TR5
    room_vertex.normal.x = 0;
    room_vertex.normal.y = 0;
    room_vertex.normal.z = 0;
    room_vertex.colour.r = room_vertex.lighting2 / 32768.0f;
    room_vertex.colour.g = room_vertex.lighting2 / 32768.0f;
    room_vertex.colour.b = room_vertex.lighting2 / 32768.0f;
    room_vertex.colour.a = 1.0f;
}

void TR_Level::read_tr2_room_staticmesh(SDL_RWops * const src, tr2_room_staticmesh_t & room_static_mesh)
{
    read_tr_vertex32(src, room_static_mesh.pos);
    room_static_mesh.rotation = (float)read_bitu16(src) / 16384.0f * -90;
    room_static_mesh.intensity1 = read_bit16(src);
    room_static_mesh.intensity2 = read_bit16(src);
    room_static_mesh.object_id = read_bitu16(src);
    // make consistent
    if (room_static_mesh.intensity1 >= 0)
        room_static_mesh.intensity1 = (8191 - room_static_mesh.intensity1) << 2;
    if (room_static_mesh.intensity2 >= 0)
        room_static_mesh.intensity2 = (8191 - room_static_mesh.intensity2) << 2;

    room_static_mesh.tint.b = room_static_mesh.tint.g = room_static_mesh.tint.r = (room_static_mesh.intensity2 / 16384.0f);
    room_static_mesh.tint.a = 1.0f;
}

void TR_Level::read_tr2_room(SDL_RWops * const src, tr5_room_t & room)
{
    uint32_t num_data_words;
    uint32_t i;
    int64_t pos;

    // read and change coordinate system
    room.offset.x = (float)read_bit32(src);
    room.offset.y = 0;
    room.offset.z = (float)-read_bit32(src);
    room.y_bottom = (float)-read_bit32(src);
    room.y_top = (float)-read_bit32(src);

    num_data_words = read_bitu32(src);

    pos = SDL_RWseek(src, 0, RW_SEEK_CUR);

    room.num_layers = 0;

    room.num_vertices = read_bitu16(src);
    room.vertices = (tr5_room_vertex_t*)calloc(room.num_vertices, sizeof(tr5_room_vertex_t));
    for (i = 0; i < room.num_vertices; i++)
        read_tr2_room_vertex(src, room.vertices[i]);

    room.num_rectangles = read_bitu16(src);
    room.rectangles = (tr4_face4_t*)malloc(room.num_rectangles * sizeof(tr4_face4_t));
    for (i = 0; i < room.num_rectangles; i++)
        read_tr_face4(src, room.rectangles[i]);

    room.num_triangles = read_bitu16(src);
    room.triangles = (tr4_face3_t*)malloc(room.num_triangles * sizeof(tr4_face3_t));
    for (i = 0; i < room.num_triangles; i++)
        read_tr_face3(src, room.triangles[i]);

    room.num_sprites = read_bitu16(src);
    room.sprites = (tr_room_sprite_t*)malloc(room.num_sprites * sizeof(tr_room_sprite_t));
    for (i = 0; i < room.num_sprites; i++)
        read_tr_room_sprite(src, room.sprites[i]);

    // set to the right position in case that there is some unused data
    SDL_RWseek(src, pos + (num_data_words * 2), RW_SEEK_SET);

    room.num_portals = read_bitu16(src);
    room.portals = (tr_room_portal_t*)malloc(room.num_portals * sizeof(tr_room_portal_t));
    for (i = 0; i < room.num_portals; i++)
        read_tr_room_portal(src, room.portals[i]);

    room.num_zsectors = read_bitu16(src);
    room.num_xsectors = read_bitu16(src);
    room.sector_list = (tr_room_sector_t*)malloc(room.num_zsectors * room.num_xsectors * sizeof(tr_room_sector_t));
    for (i = 0; i < (uint32_t)(room.num_zsectors * room.num_xsectors); i++)
        read_tr_room_sector(src, room.sector_list[i]);

    // read and make consistent
    room.intensity1 = (8191 - read_bit16(src)) << 2;
    room.intensity2 = (8191 - read_bit16(src)) << 2;
    room.light_mode = read_bit16(src);

    room.num_lights = read_bitu16(src);
    room.lights = (tr5_room_light_t*)malloc(room.num_lights * sizeof(tr5_room_light_t));
    for (i = 0; i < room.num_lights; i++)
        read_tr2_room_light(src, room.lights[i]);

    room.num_static_meshes = read_bitu16(src);
    room.static_meshes = (tr2_room_staticmesh_t*)malloc(room.num_static_meshes * sizeof(tr2_room_staticmesh_t));
    for (i = 0; i < room.num_static_meshes; i++)
        read_tr2_room_staticmesh(src, room.static_meshes[i]);

    room.alternate_room  = read_bit16(src);
    room.alternate_group = 0;   // Doesn't exist in TR1-3

    room.flags = read_bitu16(src);

    if(room.flags & 0x0020)
    {
        room.reverb_info = 0;
    }
    else
    {
        room.reverb_info = 2;
    }

    room.light_colour.r = room.intensity1 / 16384.0f;
    room.light_colour.g = room.intensity1 / 16384.0f;
    room.light_colour.b = room.intensity1 / 16384.0f;
    room.light_colour.a = 1.0f;
}

void TR_Level::read_tr2_item(SDL_RWops * const src, tr2_item_t & item)
{
    item.object_id = read_bit16(src);
    item.room = read_bit16(src);
    read_tr_vertex32(src, item.pos);
    item.rotation = (float)read_bitu16(src) / 16384.0f * -90;
    item.intensity1 = read_bitu16(src);
    if (item.intensity1 >= 0)
        item.intensity1 = (8191 - item.intensity1) << 2;
    item.intensity2 = read_bitu16(src);
    if (item.intensity2 >= 0)
        item.intensity2 = (8191 - item.intensity2) << 2;
    item.ocb = 0;   // Not present in TR2!
    item.flags = read_bitu16(src);
}

void TR_Level::read_tr2_level(SDL_RWops * const src, bool demo)
{
    uint32_t i;

    // Version
    uint32_t file_version = read_bitu32(src);

    if (file_version != 0x0000002d)
        Sys_extError("Wrong level version");

    read_tr_palette(src, this->palette);
    read_tr2_palette16(src, this->palette16);

    this->num_textiles = 0;
    this->num_room_textiles = 0;
    this->num_obj_textiles = 0;
    this->num_bump_textiles = 0;
    this->num_misc_textiles = 0;
    this->read_32bit_textiles = false;

    this->textile8_count = this->num_textiles = read_bitu32(src);
    this->textile8 = (tr_textile8_t*)malloc(this->textile8_count * sizeof(tr_textile8_t));
    for (i = 0; i < this->textile8_count; i++)
        read_tr_textile8(src, this->textile8[i]);
    this->textile16_count = this->textile8_count;
        this->textile16 = (tr2_textile16_t*)malloc(this->textile16_count * sizeof(tr2_textile16_t));
    for (i = 0; i < this->textile16_count; i++)
        read_tr2_textile16(src, this->textile16[i]);

    // Unused
    if (read_bitu32(src) != 0)
        Sys_extWarn("Bad value for 'unused'");

    this->rooms_count = read_bitu16(src);
    this->rooms = (tr5_room_t*)calloc(this->rooms_count, sizeof(tr5_room_t));
    for (i = 0; i < this->rooms_count; i++)
        read_tr2_room(src, this->rooms[i]);

    this->floor_data_size = read_bitu32(src);
    this->floor_data = (uint16_t*)malloc(this->floor_data_size * sizeof(uint16_t));
    for(i = 0; i < this->floor_data_size; i++)
        this->floor_data[i] = read_bitu16(src);

    read_mesh_data(src);

    this->animations_count = read_bitu32(src);
    this->animations = (tr_animation_t*)malloc(this->animations_count * sizeof(tr_animation_t));
    for (i = 0; i < this->animations_count; i++)
        read_tr_animation(src, this->animations[i]);

    this->state_changes_count = read_bitu32(src);
    this->state_changes = (tr_state_change_t*)malloc(this->state_changes_count * sizeof(tr_state_change_t));
    for (i = 0; i < this->state_changes_count; i++)
        read_tr_state_changes(src, this->state_changes[i]);

    this->anim_dispatches_count = read_bitu32(src);
    this->anim_dispatches = (tr_anim_dispatch_t*)malloc(this->anim_dispatches_count * sizeof(tr_anim_dispatch_t));
    for (i = 0; i < this->anim_dispatches_count; i++)
        read_tr_anim_dispatches(src, this->anim_dispatches[i]);

    this->anim_commands_count = read_bitu32(src);
    this->anim_commands = (int16_t*)malloc(this->anim_commands_count * sizeof(int16_t));
    for (i = 0; i < this->anim_commands_count; i++)
        this->anim_commands[i] = read_bit16(src);

    this->mesh_tree_data_size = read_bitu32(src);
    this->mesh_tree_data = (uint32_t*)malloc(this->mesh_tree_data_size * sizeof(uint32_t));
    for (i = 0; i < this->mesh_tree_data_size; i++)
        this->mesh_tree_data[i] = read_bitu32(src);                     // 4 bytes

    read_frame_moveable_data(src);

    this->static_meshes_count = read_bitu32(src);
    this->static_meshes = (tr_staticmesh_t*)malloc(this->static_meshes_count * sizeof(tr_staticmesh_t));
    for (i = 0; i < this->static_meshes_count; i++)
        read_tr_staticmesh(src, this->static_meshes[i]);

    this->object_textures_count = read_bitu32(src);
    this->object_textures = (tr4_object_texture_t*)malloc(this->object_textures_count * sizeof(tr4_object_texture_t));
    for (i = 0; i < this->object_textures_count; i++)
        read_tr_object_texture(src, this->object_textures[i]);

    this->sprite_textures_count = read_bitu32(src);
    this->sprite_textures = (tr_sprite_texture_t*)malloc(this->sprite_textures_count * sizeof(tr_sprite_texture_t));
    for (i = 0; i < this->sprite_textures_count; i++)
        read_tr_sprite_texture(src, this->sprite_textures[i]);

    this->sprite_sequences_count = read_bitu32(src);
    this->sprite_sequences = (tr_sprite_sequence_t*)malloc(this->sprite_sequences_count * sizeof(tr_sprite_sequence_t));
    for (i = 0; i < this->sprite_sequences_count; i++)
        read_tr_sprite_sequence(src, this->sprite_sequences[i]);

    if (demo)
        read_tr_lightmap(src, this->lightmap);

    this->cameras_count = read_bitu32(src);
    this->cameras = (tr_camera_t*)malloc(this->cameras_count * sizeof(tr_camera_t));
    for (i = 0; i < this->cameras_count; i++)
    {
        this->cameras[i].x = read_bit32(src);
        this->cameras[i].y = read_bit32(src);
        this->cameras[i].z = read_bit32(src);

        this->cameras[i].room = read_bit16(src);
        this->cameras[i].unknown1 = read_bitu16(src);
    }

    this->sound_sources_count = read_bitu32(src);
    this->sound_sources = (tr_sound_source_t*)malloc(this->sound_sources_count * sizeof(tr_sound_source_t));
    for(i = 0; i < this->sound_sources_count; i++)
    {
        this->sound_sources[i].x = read_bit32(src);
        this->sound_sources[i].y = read_bit32(src);
        this->sound_sources[i].z = read_bit32(src);

        this->sound_sources[i].sound_id = read_bitu16(src);
        this->sound_sources[i].flags = read_bitu16(src);
    }

    this->boxes_count = read_bitu32(src);
    this->boxes = (tr_box_t*)malloc(this->boxes_count * sizeof(tr_box_t));
    this->zones = (tr2_zone_t*)malloc(this->boxes_count * sizeof(tr2_zone_t));
    for (i = 0; i < this->boxes_count; i++)
        read_tr2_box(src, this->boxes[i]);

    this->overlaps_count = read_bitu32(src);
    this->overlaps = (uint16_t*)malloc(this->overlaps_count * sizeof(uint16_t));
    for (i = 0; i < this->overlaps_count; i++)
        this->overlaps[i] = read_bitu16(src);

    // Zones
    for (i = 0; i < this->boxes_count; i++)
        read_tr2_zone(src, this->zones[i]);

    this->animated_textures_count = read_bitu32(src);
    this->animated_textures_uv_count = 0; // No UVRotate in TR2
    this->animated_textures = (uint16_t*)malloc(this->animated_textures_count * sizeof(uint16_t));
    for (i = 0; i < this->animated_textures_count; i++)
    {
        this->animated_textures[i] = read_bitu16(src);
    }

    this->items_count = read_bitu32(src);
    this->items = (tr2_item_t*)malloc(this->items_count * sizeof(tr2_item_t));
    for (i = 0; i < this->items_count; i++)
        read_tr2_item(src, this->items[i]);

    if (!demo)
        read_tr_lightmap(src, this->lightmap);

    this->cinematic_frames_count = read_bitu16(src);
    this->cinematic_frames = (tr_cinematic_frame_t*)malloc(this->cinematic_frames_count * sizeof(tr_cinematic_frame_t));
    for (i = 0; i < this->cinematic_frames_count; i++)
    {
        read_tr_cinematic_frame(src, this->cinematic_frames[i]);
    }

    this->demo_data_count = read_bitu16(src);
    this->demo_data = (uint8_t*)malloc(this->demo_data_count * sizeof(uint8_t));
    for(i=0; i < this->demo_data_count; i++)
        this->demo_data[i] = read_bitu8(src);

    // Soundmap
    this->soundmap = (int16_t*)malloc(TR_AUDIO_MAP_SIZE_TR2 * sizeof(int16_t));
    for(i=0; i < TR_AUDIO_MAP_SIZE_TR2; i++)
        this->soundmap[i] = read_bit16(src);

    this->sound_details_count = read_bitu32(src);
    this->sound_details = (tr_sound_details_t*)malloc(this->sound_details_count * sizeof(tr_sound_details_t));

    for(i = 0; i < this->sound_details_count; i++)
    {
        this->sound_details[i].sample = read_bitu16(src);
        this->sound_details[i].volume = read_bitu16(src);
        this->sound_details[i].chance = read_bitu16(src);
        this->sound_details[i].num_samples_and_flags_1 = read_bitu8(src);
        this->sound_details[i].flags_2 = read_bitu8(src);
        this->sound_details[i].sound_range = TR_AUDIO_DEFAULT_RANGE;
        this->sound_details[i].pitch = (int16_t)TR_AUDIO_DEFAULT_PITCH;
    }

    this->sample_indices_count = read_bitu32(src);
    this->sample_indices = (uint32_t*)malloc(this->sample_indices_count * sizeof(uint32_t));
    for(i=0; i < this->sample_indices_count; i++)
        this->sample_indices[i] = read_bitu32(src);

    // remap all sample indices here
    for(i = 0; i < this->sound_details_count; i++)
    {
        if(this->sound_details[i].sample < this->sample_indices_count)
        {
            this->sound_details[i].sample = this->sample_indices[this->sound_details[i].sample];
        }
    }

    // LOAD SAMPLES

    // In TR2, samples are stored in separate file called MAIN.SFX.
    // If there is no such files, no samples are loaded.

    SDL_RWops *newsrc = SDL_RWFromFile(this->sfx_path, "rb");
    if (newsrc == NULL)
    {
        Sys_extWarn("read_tr2_level: failed to open \"%s\"! No samples loaded.", this->sfx_path);
    }
    else
    {
        this->samples_data_size = SDL_RWsize(newsrc);
        this->samples_count = 0;
        this->samples_data = (uint8_t*)malloc(this->samples_data_size * sizeof(uint8_t));
        for(i = 0; i < this->samples_data_size; i++)
        {
            this->samples_data[i] = read_bitu8(newsrc);
            if((i >= 4) && (*((uint32_t*)(this->samples_data+i-4)) == 0x46464952))   /// RIFF
            {
                this->samples_count++;
            }
        }

        SDL_RWclose(newsrc);
        newsrc = NULL;
    }
}
