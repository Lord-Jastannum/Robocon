import asyncio
import websockets
import pygame
import json

pygame.init()

WIDTH, HEIGHT = 2000, 1000
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Mouse Movement from Serial")

# Colors
WHITE = (255, 255, 255)
RED = (255, 0, 0)
BLACK = (0, 0, 0)

trail = []
xc, yc = 0, 0

# WebSocket Server Address
WS_URL = "ws://192.168.222.240:5000"

# update final destination

async def send_destination():
    async with websockets.connect(WS_URL) as websocket:
        while True:
            xf = int(input("Enter X:"))
            #yf = int(input("Enter Y:"))
            yf = 20

            data = {"x": xf, "y": yf, "angle": 45}
            await websockets.send(json.dumps(data))
            response = await websocket.recv()
            print("Response:", response)

# receive and update bot's current position

async def receive_data():
    global xc, yc
    async with websockets.connect(WS_URL) as websockets:
        async for message in websockets:
            data = json.loads(message)
            xc, yc = data.get("x", 0), data.get("y", 0)

async def run_visualization():
    global xc, yc
    running = True
    while running:
        screen.fill(WHITE)

        if xc:
            x, y = xc/3, yc/3
            trail.append((WIDTH // 2+x, HEIGHT // 2-y))

        if len(trail) > 1:
            pygame.draw.lines(screen, RED, False, trail, 3)

        if trail:
            font = pygame.font.Font(None, 36)
            text = font.render(f"Position: {trail[-1]}", True, BLACK)
            screen.blit(text, (10, 10))

        pygame.display.flip()

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

    pygame.quit()

async def main():
    await asyncio.gather(receive_data, send_destination(), run_visualization())


if __name__ == '__main__':
    asyncio.run(main())