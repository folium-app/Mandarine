//
//  Mandarine.swift
//  Mandarine
//
//  Created by Jarrod Norwell on 17/11/2025.
//

@objcMembers
public class MandarineCommon : NSObject {
    public static var documentDirectoryURL: URL? {
        FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first
    }
    
    public static var mandarineDirectoryURL: URL? {
        if let documentDirectoryURL {
            documentDirectoryURL.appending(component: "Mandarine")
        } else {
            nil
        }
    }
    
    public static var memcardsDirectoryURL: URL? {
        if let mandarineDirectoryURL {
            mandarineDirectoryURL.appending(component: "memcards")
        } else {
            nil
        }
    }
    
    public static var sysdataDirectoryURL: URL? {
        if let mandarineDirectoryURL {
            mandarineDirectoryURL.appending(component: "sysdata")
        } else {
            nil
        }
    }
}

public actor Mandarine {
    public let emulator: MandarineEmulator = MandarineEmulator.shared()
    
    public init() {}
    
    
    public func insert(cartridge: URL) {
        emulator.insert(cartridge: cartridge)
    }
    
    
    public func pause() {
        emulator.pause()
    }
    
    public func start() {
        emulator.start()
    }
    
    public func stop() {
        emulator.stop()
    }
    
    public func unpause() {
        emulator.unpause()
    }
    
    
    public var paused: Bool {
        emulator.paused()
    }
    
    public var running: Bool {
        emulator.running()
    }
    
    
    public func press(button: String) {
        emulator.press(button)
    }
    
    public func release(button: String) {
        emulator.release(button)
    }
    
    
    public func load(state: URL) {
        emulator.load(state: state)
    }
    
    public func save(state: URL) {
        emulator.save(state: state)
    }
    
    
    public func identifier(cartridge: URL) -> String {
        emulator.identifier(cartridge: cartridge)
    }
    
    
    public func audioCallback(output: @escaping (UnsafeMutablePointer<UInt16>, Int) -> Void) {
        emulator.audioCallback = output
    }
    
    public func videoCallback(output: @escaping (UnsafeMutableRawPointer, Int, Int, Int, Int) -> Void) { // BGR555
        emulator.videoCallback = output
    }
    
    public func secondaryVideoCallback(output: @escaping (UnsafeMutablePointer<UInt16>, Int, Int, Int, Int) -> Void) { // RGB24
        emulator.secondaryVideoCallback = output
    }
}

/*
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
*/
