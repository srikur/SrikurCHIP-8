//
//  ViewController.swift
//  SrikurCHIP-8
//
//  Created by Srikur Kanuparthy on 6/10/24.
//

import Cocoa
import SpriteKit
import GameplayKit

class ViewController: NSViewController {

    @IBOutlet var skView: SKView!
    
    override func viewDidLoad() {
        super.viewDidLoad()

        if let view = self.skView {
            let scene = Emulator()
            // Set the scale mode to scale to fit the window
            scene.scaleMode = .aspectFit
            
            scene.view?.preferredFramesPerSecond = 60
            
            // Present the scene
            view.presentScene(scene)
            
            view.ignoresSiblingOrder = true
            
            view.showsFPS = true
            view.showsNodeCount = true
        }
    }
}

