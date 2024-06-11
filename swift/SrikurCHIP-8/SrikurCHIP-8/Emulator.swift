//
//  GameScene.swift
//  SrikurCHIP-8
//
//  Created by Srikur Kanuparthy on 6/10/24.
//

import SpriteKit
import GameplayKit

class Emulator: SKScene {
    
    private var textureNode : SKSpriteNode!
    private var pixelArray : [Bool] = Array(repeating: false, count: 2048)
    private var cpu : CPU = CPU(ipf: 9, romPath: "/Users/srikur/Documents/GitHub/SrikurCHIP-8/roms/tests/1-chip8-logo.ch8")
    private var quit : Bool = false
    private let keycodeMap : [Int: Int] = [
        0x07: 0x00,
        0x18: 0x01,
        0x19: 0x02,
        0x20: 0x03,
        0x12: 0x04,
        0x13: 0x05,
        0x14: 0x06,
        0x00: 0x07,
        0x01: 0x08,
        0x02: 0x09,
        0x06: 0x0A,
        0x08: 0x0B,
        0x21: 0x0C,
        0x15: 0x0D,
        0x03: 0x0E,
        0x09: 0x0F
    ]
    
    required init?(coder aDecoder: NSCoder) {
        self.cpu = CPU(ipf: 9, romPath: "/Users/srikur/Documents/GitHub/SrikurCHIP-8/roms/tests/1-chip8-logo.ch8")
        super.init(coder: aDecoder)
    }
    
    override func didMove(to view: SKView) {
        textureNode = SKSpriteNode(texture: createTexture())
        textureNode.size = CGSize(width: 640, height: 320)
        textureNode.position = CGPoint(x: frame.midX, y: frame.midY)
        addChild(textureNode)
    }
    
    func createTexture() -> SKTexture {
        // Retrieve pixel array from CPU and create a texture
        let bytesPerPixel = 4
        let bitsPerComponent = 8
        var pixelData = cpu.getScreen()
        let providerRef = CGDataProvider(data: NSData(bytes: &pixelData, length: pixelData.count * MemoryLayout<UInt8>.size))
        let cgImage = CGImage(
            width: CPU.SCREEN_WIDTH,
            height: CPU.SCREEN_HEIGHT,
            bitsPerComponent: bitsPerComponent,
            bitsPerPixel: bitsPerComponent * bytesPerPixel,
            bytesPerRow: CPU.SCREEN_WIDTH * bytesPerPixel,
            space: CGColorSpaceCreateDeviceRGB(),
            bitmapInfo: CGBitmapInfo(rawValue: CGImageAlphaInfo.premultipliedLast.rawValue),
            provider: providerRef!,
            decode: nil,
            shouldInterpolate: false,
            intent: .defaultIntent
        )

        return SKTexture(cgImage: cgImage!)
    }
    
    override func keyDown(with event: NSEvent) {
        if let keycode = keycodeMap[Int(event.keyCode)] {
            cpu.setKey(keycode, true)
        }
    }

    override func keyUp(with event: NSEvent) {
        if let keycode = keycodeMap[Int(event.keyCode)] {
            cpu.setKey(keycode, false)
        }
    }
    
    override func update(_ currentTime: TimeInterval) {
        // Called before each frame is rendered
        cpu.emulateCycle()
        
        if cpu.drawFlag {
            textureNode.texture = createTexture()
        }
    }
}
