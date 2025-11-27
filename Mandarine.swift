//
//  Mandarine.swift
//  Mandarine
//
//  Created by Jarrod Norwell on 17/11/2025.
//

public typealias BufferHandler = (UnsafeMutableRawPointer, Int, Int, Int, Int) -> Void
public typealias BGR555Handler = BufferHandler
public typealias RGB888Handler = (UnsafeMutablePointer<UInt16>, Int, Int, Int, Int) -> Void

public enum PSXButton : String {
    case up = "dpad_up",
         right = "dpad_right",
         down = "dpad_down",
         left = "dpad_left",
         triangle = "triangle",
         circle = "circle",
         cross = "cross",
         square = "square",
         select = "select",
         start = "start",
         l1 = "l1",
         r1 = "r1",
         l2 = "l2",
         r2 = "r2",
         
         lUp = "l_up",
         lRight = "l_right",
         lDown = "l_down",
         lLeft = "l_left",
         
         rUp = "r_up",
         rRight = "r_right",
         rDown = "r_down",
         rLeft = "r_left"
    
    var string: String { rawValue }
}

public class Mandarine {
    private var emulator: MandarineEmulator = .shared()
    
    public init() {}
    
    public func insert(_ cartridge: URL) {
        emulator.insert(cartridge)
    }
    
    public func start() {
        emulator.start()
    }
    
    public func stop() {
        emulator.stop()
    }
    
    public func pause(_ pause: Bool) {
        emulator.pause(pause)
    }
    
    public var isPaused: Bool {
        get {
            emulator.isPaused()
        }
        set {
            pause(newValue)
        }
    }
    
    public func bgr555(_ buffer: @escaping BGR555Handler) {
        emulator.bgr555 = buffer
    }
    
    public func rgb888(_ buffer: @escaping RGB888Handler) {
        emulator.rgb888 = buffer
    }
    
    public func button(button: PSXButton, player: Int, pressed: Bool) {
        emulator.input(player, button: button.string, pressed: pressed)
    }
    
    public func drag(_ slot: Int, _ stick: PSXButton, _ value: Int16) {
        emulator.drag(slot, stick: stick.string, value: value)
    }
    
    public func load(state url: URL) {
        emulator.load(url)
    }
    
    public func save(state url: URL) {
        emulator.save(url)
    }
    
    public func id(from url: URL) -> String {
        emulator.id(from: url).replacingOccurrences(of: "_", with: "-").replacingOccurrences(of: ".", with: "")
    }
}
