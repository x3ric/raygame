#!/bin/ny
#include <raylib.h>
#include <rlgl.h> as "rl"

use std.core
use std.core.mem as mem
use std.core.str as str
use std.os.args as cli
use std.os.ffi as ffi
use std.math as math

def COLOR_BG         = 0xff000000
def COLOR_TILE_A     = 0xff3e2650
def COLOR_TILE_B     = 0xff000000
def COLOR_TILE_LINE  = 0xff201527
def COLOR_PLAYER     = 0xfffff8f8
def COLOR_PLAYER_ALT = 0xffff00ff
def COLOR_TEXT       = 0xffffffff
def COLOR_DIM        = 0xff989898
def COLOR_ACCENT     = 0xffff00ff
def CHECKED_CELL     = 16
def PLAYER_HALF      = 64
def BASE_SPEED       = 135.0
def SPRINT_MULT      = 1.55
def CAMERA_LERP      = 1.25
def DIAGONAL_LEAN    = 10.0
def ROTATION_SPEED   = 180.0
def ANIM_IDLE_LEFT   = 0
def ANIM_IDLE_RIGHT  = 1
def ANIM_IDLE_DOWN   = 2
def ANIM_IDLE_UP     = 3
def ANIM_MOVE_LEFT   = 4
def ANIM_MOVE_RIGHT  = 5
def ANIM_MOVE_DOWN   = 6
def ANIM_MOVE_UP     = 7
def ANIM_NAMES = [
   "IDLE_LEFT", "IDLE_RIGHT", "IDLE_DOWN", "IDLE_UP",
   "MOVE_LEFT", "MOVE_RIGHT", "MOVE_DOWN", "MOVE_UP"
]
def ANIM_FILES = [
   "idle.gif", "idle.gif", "idle-down.gif", "idle-up.gif",
   "move.gif", "move.gif", "move-down.gif", "move-up.gif"
]

fn ray_color(packed){
   Color((packed >> 16) & 255, (packed >> 8) & 255, packed & 255, (packed >> 24) & 255)
}

fn color_blend(a, b, t){ t > 0.5 ? b : a }

fn key_down(a, b){ IsKeyDown(a) || IsKeyDown(b) }

fn move_axis(positive, negative){ positive == negative ? 0 : (positive ? 1 : -1) }

fn input_axis(){
   [
      move_axis(key_down(KEY_RIGHT, KEY_D), key_down(KEY_LEFT, KEY_A)),
      move_axis(key_down(KEY_DOWN, KEY_S), key_down(KEY_UP, KEY_W))
   ]
}

fn input_anim(axis_x, axis_y, last_idle, flipped){
   mut anim = last_idle
   if(axis_x > 0){
      anim = ANIM_MOVE_RIGHT
      last_idle = ANIM_IDLE_RIGHT
      flipped = false
   } elif(axis_x < 0){
      anim = ANIM_MOVE_LEFT
      last_idle = ANIM_IDLE_LEFT
      flipped = true
   }
   if(axis_y < 0){
      anim = ANIM_MOVE_UP
      last_idle = ANIM_IDLE_UP
   } elif(axis_y > 0){
      anim = ANIM_MOVE_DOWN
      last_idle = ANIM_IDLE_DOWN
   }
   [(axis_x == 0 && axis_y == 0) ? last_idle : anim, last_idle, flipped]
}

fn sprint_state(speed, rumblespeed, frame_duration){
   if(IsKeyPressed(KEY_LEFT_SHIFT)){
      speed = (speed + rumblespeed) * SPRINT_MULT
      frame_duration /= 2.0
   }
   if(IsKeyReleased(KEY_LEFT_SHIFT)){
      speed = (speed + rumblespeed) / SPRINT_MULT
      frame_duration *= 2.0
      rumblespeed = 0.0
   }
   [speed, rumblespeed, frame_duration]
}

fn move_rotation(current, dt, axis_x, axis_y){
   def target = (axis_x != 0 && axis_y != 0) ? -float(axis_x * axis_y) * DIAGONAL_LEAN : 0.0
   math.clamp(target, current - ROTATION_SPEED * dt, current + ROTATION_SPEED * dt)
}

fn anim_name(anim){ ANIM_NAMES.get(anim, "UNKNOWN") }

fn anim_path(root, anim){ root + "/res/sprites/player/" + ANIM_FILES.get(anim, "idle.gif") }

fn image_data(img){ img ? load64(img, __layout_offset("Image", "data")) : 0 }

fn image_width(img){ img ? load_layout(img, "Image", "width") : 0 }

fn image_height(img){ img ? load_layout(img, "Image", "height") : 0 }

fn image_frame_bytes(img){ image_width(img) * image_height(img) * 4 }

fn image_frame_ptr(img, frames, frame){
   def data = image_data(img)
   if(!data){ return 0 }
   data + image_frame_bytes(img) * (frames > 0 ? frame % frames : 0)
}

fn image_view(img, data){
   def view = malloc(__layout_size("Image"))
   store_layout(
      view,
      "Image",
      data,
      image_width(img),
      image_height(img),
      1,
      load_layout(img, "Image", "format")
   )
   view
}

fn mirror_frame(src, dst, width, height){
   if(!src || !dst || width <= 0 || height <= 0){ return 0 }
   mut y = 0
   while(y < height){
      mut x = 0
      while(x < width){
         def sp, dp = (y * width + (width - 1 - x)) * 4, (y * width + x) * 4
         store32(dst, load32(src, sp), dp)
         x += 1
      }
      y += 1
   }
   dst
}

fn frame_pixels(img, frame_buf, frame, frames, flipped){
   def src = image_frame_ptr(img, frames, frame)
   if(!src || !frame_buf){ return 0 }
   if(flipped){ return mirror_frame(src, frame_buf, image_width(img), image_height(img)) }
   mem.memcpy(frame_buf, src, image_frame_bytes(img))
   frame_buf
}

fn unload_anim(img, tex, frame_buf){
   if(tex){
      UnloadTexture(tex)
      free(tex)
   }
   if(frame_buf){ free(frame_buf) }
   if(img){
      UnloadImage(img)
      free(img)
   }
}

fn reload_anim(root, anim, frames_ptr, img, tex, frame_buf){
   unload_anim(img, tex, frame_buf)
   def next = load_anim(root, anim, frames_ptr)
   [next[0], next[1], next[2], load32(frames_ptr, 0)]
}

fn load_anim(root, anim, frames_ptr): any {
   store32(frames_ptr, 0, 0)
   def img = LoadImageAnim(ffi.cptr(anim_path(root, anim)), frames_ptr)
   if(!img || !image_data(img)){
      if(img){ free(img) }
      return [0, 0, 0]
   }
   def frames = load32(frames_ptr, 0)
   def view = image_view(img, image_data(img))
   def tex = LoadTextureFromImage(view)
   free(view)
   [img, tex, malloc(image_frame_bytes(img))]
}

fn update_anim_frame(img, frames, current_frame, frame_counter, frame_duration, dt): any {
   frame_counter += dt
   if(img && frames > 0 && frame_counter >= frame_duration){
      current_frame += 1
      if(current_frame >= frames){ current_frame = 0 }
      frame_counter = 0.0
   }
   [current_frame, frame_counter]
}

fn update_pulse(pulse, dir, dt, sprinting){
   pulse += dir * dt * (sprinting ? 4.0 : 2.0)
   if(pulse > 1.0){ return [1.0, -1.0] }
   if(pulse < 0.0){ return [0.0, 1.0] }
   [pulse, dir]
}

fn update_camera(cam_x, cam_y, px, py, dt){
   def t = math.clamp(CAMERA_LERP * dt, 0.0, 1.0)
   [math.lerp(cam_x, px, t), math.lerp(cam_y, py, t)]
}

fn world_to_screen(x, y, cam_x, cam_y, sw, sh){
   [
      int(math.floor(x - cam_x + float(sw) * 0.5)),
      int(math.floor(y - cam_y + float(sh) * 0.5))
   ]
}

fn draw_checker(cam_x, cam_y, sw, sh): any {
   def tile_a, tile_b = ray_color(COLOR_TILE_A), ray_color(COLOR_TILE_B)
   def cell = float(CHECKED_CELL)
   def world_left, world_top = cam_x - float(sw) * 0.5, cam_y - float(sh) * 0.5
   def start_col, start_row = int(math.floor(world_left / cell)) - 1, int(math.floor(world_top / cell)) - 1
   def end_col = int(math.floor((world_left + float(sw)) / cell)) + 2
   def end_row = int(math.floor((world_top + float(sh)) / cell)) + 2
   mut row = start_row
   while(row <= end_row){
      def y0, y1 = int(math.floor(float(row) * cell - world_top)), int(math.floor(float(row + 1) * cell - world_top))
      mut col = start_col
      while(col <= end_col){
         def x0 = int(math.floor(float(col) * cell - world_left))
         def x1 = int(math.floor(float(col + 1) * cell - world_left))
         DrawRectangle(
            x0,
            y0,
            int(math.max(1, x1 - x0)),
            int(math.max(1, y1 - y0)),
            ((row + col) % 2 == 0) ? tile_a : tile_b
         )
         col += 1
      }
      row += 1
   }
   free(tile_a, tile_b) return 0
}

fn draw_crosshair(sw, sh): any {
   def line = ray_color(COLOR_TILE_LINE)
   def cx, cy = sw / 2, sh / 2
   DrawLine(cx - 12, cy, cx - 4, cy, line)
   DrawLine(cx + 4, cy, cx + 12, cy, line)
   DrawLine(cx, cy - 12, cx, cy - 4, line)
   DrawLine(cx, cy + 4, cx, cy + 12, line)
   free(line) return 0
}

fn aim_vector(anim, flipped){
   match(anim){
      ANIM_MOVE_RIGHT, ANIM_IDLE_RIGHT -> [46, 0]
      ANIM_MOVE_LEFT, ANIM_IDLE_LEFT -> [-46, 0]
      ANIM_MOVE_DOWN, ANIM_IDLE_DOWN -> [0, 46]
      ANIM_MOVE_UP, ANIM_IDLE_UP -> [0, -46]
      _ -> [flipped ? -46 : 46, 0]
   }
}

fn draw_player_fallback(px, py, cam_x, cam_y, sw, sh, anim, flipped, rotation, pulse): any {
   def pos = world_to_screen(px, py, cam_x, cam_y, sw, sh)
   def cx, cy = pos[0], pos[1]
   def bg = ray_color(COLOR_BG)
   def body = color_blend(COLOR_PLAYER, COLOR_PLAYER_ALT, pulse)
   def body_c = ray_color(body)
   def text = ray_color(COLOR_TEXT)
   def accent = ray_color(COLOR_ACCENT)
   DrawRectangle(cx - PLAYER_HALF, cy - PLAYER_HALF, PLAYER_HALF * 2, PLAYER_HALF * 2, body_c)
   DrawRectangleLines(cx - PLAYER_HALF, cy - PLAYER_HALF, PLAYER_HALF * 2, PLAYER_HALF * 2, text)
   def aim = aim_vector(anim, flipped)
   def ax, ay = aim[0], aim[1]
   DrawLine(cx, cy, cx + ax, cy + ay, bg)
   DrawCircle(cx + ax, cy + ay, 8.0, bg)
   if(rotation != 0.0){
      def lean = int(rotation * 2.0)
      DrawLine(
         cx - PLAYER_HALF,
         cy + PLAYER_HALF + 8,
         cx + PLAYER_HALF,
         cy + PLAYER_HALF + 8 + lean,
         accent
      )
   }
   free(bg, body_c, text, accent) return 0
}

fn update_player_texture(img, tex, frame_buf, frame, frames, flipped){
   if(!img || !tex){ return false }
   def data = frame_pixels(img, frame_buf, frame, frames, flipped)
   if(!data){ return false }
   UpdateTexture(tex, data)
   true
}

fn draw_player_texture(tex, px, py, cam_x, cam_y, sw, sh, rotation){
   if(!tex){ return false }
   def tw, th = load_layout(tex, "Texture2D", "width"), load_layout(tex, "Texture2D", "height")
   if(tw <= 0 || th <= 0){ return false }
   def scale = 4
   def pos = world_to_screen(px, py, cam_x, cam_y, sw, sh)
   def cx, cy = pos[0], pos[1]
   def tint = ray_color(COLOR_TEXT)
   rlPushMatrix()
   rlTranslatef(float(cx), float(cy), 0.0)
   rlRotatef(rotation, 0.0, 0.0, 1.0)
   rlScalef(float(scale), float(scale), 1.0)
   DrawTexture(tex, -(tw / 2), -(th / 2), tint)
   rlPopMatrix()
   free(tint)
   true
}

fn draw_player(tex, px, py, cam_x, cam_y, sw, sh, anim, flipped, rotation, pulse){
   if(!draw_player_texture(tex, px, py, cam_x, cam_y, sw, sh, rotation)){
      draw_player_fallback(px, py, cam_x, cam_y, sw, sh, anim, flipped, rotation, pulse)
   }
}

fn draw_hud(px, py, speed, anim, frames, sw, sh): any {
   def text = ray_color(COLOR_TEXT)
   def dim = ray_color(COLOR_DIM)
   def accent = ray_color(COLOR_ACCENT)
   DrawFPS(12, 10)
   DrawText(ffi.cptr("Nytrix Raygame"), 12, 34, 20, text)
   DrawText(ffi.cptr("WASD/arrows move  shift sprint  esc quit"), 12, 58, 14, dim)
   DrawText(ffi.cptr("x=" + to_str(int(px)) + " y=" + to_str(int(py))), 12, sh - 70, 16, text)
   DrawText(ffi.cptr("state=" + anim_name(anim) + " speed=" + to_str(int(speed))), 12, sh - 48, 16, accent)
   DrawText(ffi.cptr("frame=" + to_str(frames)), 12, sh - 26, 16, dim)
   free(text, dim, accent) return 0
}

fn draw_scene(player_img, player_tex, player_buf, anim_frame, anim_frames, flipped, px, py, cam_x, cam_y, sw, sh, anim, rotation, pulse, step, frames){
   BeginDrawing()
   def clear_color = ray_color(COLOR_BG)
   ClearBackground(clear_color)
   free(clear_color)
   draw_checker(cam_x, cam_y, sw, sh)
   draw_crosshair(sw, sh)
   update_player_texture(player_img, player_tex, player_buf, anim_frame, anim_frames, flipped)
   draw_player(player_tex, px, py, cam_x, cam_y, sw, sh, anim, flipped, rotation, pulse)
   draw_hud(px, py, step, anim, frames, sw, sh)
   EndDrawing()
}

fn should_stop(interactive, frames, max_frames, elapsed, max_seconds){
   !interactive
   && (
      (max_frames > 0 && frames >= max_frames)
      || (max_seconds > 0.0 && elapsed >= max_seconds)
   )
}

fn main(): any {
   def args = cli.args()
   def width = int(math.max(cli.int_value_from(args, "--width", 1920), 1))
   def height = int(math.max(cli.int_value_from(args, "--height", 1080), 1))
   def max_frames = int(math.max(cli.int_value_from(args, "--frames", 0), 0))
   def max_seconds = math.max(cli.float_value_from(args, "--seconds", 0.0), 0.0)
   def interactive = max_frames <= 0 && max_seconds <= 0.0
   def fps = int(math.max(cli.int_value_from(args, "--fps", 0), 0))
   def asset_root = cli.value_from(args, "--asset-root", ".")

   SetConfigFlags(FLAG_WINDOW_RESIZABLE)
   InitWindow(width, height, "raylib game")
   if(fps > 0){ SetTargetFPS(fps) }

   mut px, py, cam_x, cam_y = 0.0, 0.0, 0.0, 0.0
   mut frames, elapsed, flipped = 0, 0.0, true
   mut last_idle, current_anim, prev_anim = ANIM_IDLE_LEFT, ANIM_IDLE_LEFT, ANIM_IDLE_LEFT
   mut speed, rumblespeed, frame_duration = BASE_SPEED, 0.0, 0.25
   mut frame_counter, anim_frame = 0.0, 0
   def anim_frames_ptr = malloc(4)
   mut anim_frames, player_img, player_tex, player_buf = 0, 0, 0, 0
   def loaded = load_anim(asset_root, current_anim, anim_frames_ptr)
   player_img, player_tex, player_buf = loaded[0], loaded[1], loaded[2]
   anim_frames = load32(anim_frames_ptr, 0)
   mut rotation, pulse, pulse_dir = 0.0, 0.0, 1.0

   while(!(WindowShouldClose() != 0)){
      def dt = GetFrameTime()
      elapsed += dt
      def sw, sh = GetScreenWidth(), GetScreenHeight()
      def frame_state = update_anim_frame(player_img, anim_frames, anim_frame, frame_counter, frame_duration, dt)
      anim_frame, frame_counter = frame_state[0], frame_state[1]
      prev_anim = current_anim

      def sprint = sprint_state(speed, rumblespeed, frame_duration)
      speed, rumblespeed, frame_duration = sprint[0], sprint[1], sprint[2]

      def axis = input_axis()
      def axis_x, axis_y = axis[0], axis[1]
      def step = speed + rumblespeed
      px += float(axis_x) * step * dt
      py += float(axis_y) * step * dt
      if(axis_x == 0 && axis_y == 0){ rumblespeed = 0.0 }

      def motion = input_anim(axis_x, axis_y, last_idle, flipped)
      current_anim, last_idle, flipped = motion[0], motion[1], motion[2]
      if(prev_anim != current_anim){
         def next = reload_anim(asset_root, current_anim, anim_frames_ptr, player_img, player_tex, player_buf)
         player_img, player_tex, player_buf, anim_frames = next[0], next[1], next[2], next[3]
         anim_frame, frame_counter = 0, 0.0
      }
      rotation = move_rotation(rotation, dt, axis_x, axis_y)
      def camera = update_camera(cam_x, cam_y, px, py, dt)
      cam_x, cam_y = camera[0], camera[1]
      def pulse_state = update_pulse(pulse, pulse_dir, dt, IsKeyDown(KEY_LEFT_SHIFT))
      pulse, pulse_dir = pulse_state[0], pulse_state[1]
      draw_scene(player_img, player_tex, player_buf, anim_frame, anim_frames, flipped, px, py, cam_x, cam_y, sw, sh, current_anim, rotation, pulse, step, frames)
      frames += 1
      if(should_stop(interactive, frames, max_frames, elapsed, max_seconds)){ break }
   }
   unload_anim(player_img, player_tex, player_buf)
   CloseWindow()
}
