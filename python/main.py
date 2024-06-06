import sys
import cpu
import sdl2.ext

class Emulator:
    romPath = ""
    quit = False
    chip8 = None
    WINDOW_SCALE = 10

    def __init__(self, path: str):
        self.romPath = path
        self.quit = False
        self.chip8 = cpu.CPU()

    def create_window_and_renderer(self):
        sdl2.ext.init()
        window = sdl2.ext.Window("Srikur CHIP-8", size=(64 * self.WINDOW_SCALE, 32 * self.WINDOW_SCALE))
        window.show()
        renderer = sdl2.ext.Renderer(window)
        return renderer

    def run(self):
        renderer = self.create_window_and_renderer()
        texture = sdl2.SDL_CreateTexture(renderer.sdlrenderer, sdl2.SDL_PIXELFORMAT_ARGB8888, sdl2.SDL_TEXTUREACCESS_STREAMING, 64, 32)

        # Load the ROM
        self.chip8.load_rom(self.romPath)

        # Main loop
        while not self.quit:
            # Emulate one cycle
            self.chip8.emulateCycle()

            # Update the screen
            if self.chip8.draw_flag:

                # Get the screen buffer
                pixels = self.chip8.getScreen()
                screen_buffer = bytes(64 * 32 * 4)
                for i in range(64 * 32):
                    color = (255, 255, 255, 255) if pixels[i] != 0 else (0, 0, 0, 255)
                    offset = i * 4
                    screen_buffer = screen_buffer[:offset] + bytes(color) + screen_buffer[offset + 4:]
                sdl2.SDL_UpdateTexture(texture, None, screen_buffer, 64 * 4)

                renderer.clear()
                sdl2.SDL_RenderCopy(renderer.sdlrenderer, texture, None, None)
                renderer.present()

                self.chip8.draw_flag = False

            events = sdl2.ext.get_events()
            for event in events:
                # Check if the event is a quit event or ESC key
                if event.type == sdl2.SDL_QUIT or (event.type == sdl2.SDL_KEYDOWN and event.key.keysym.sym == sdl2.SDLK_ESCAPE):
                    self.quit = True
                    break

        sdl2.SDL_DestroyTexture(texture)
        sdl2.ext.quit()

# emu = Emulator("../roms/tests/1-chip8-logo.ch8")
# emu = Emulator("../roms/tests/2-ibm-logo.ch8")
# emu = Emulator("../roms/tests/3-corax+.ch8")
# emu = Emulator("../roms/tests/4-flags.ch8")
emu = Emulator("../roms/INVADERS")
emu.run()