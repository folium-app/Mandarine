//
//  ID.swift
//  Mandarine
//
//  Created by Jarrod Norwell on 1/10/2026.
//

import Foundation

public enum IDError: LocalizedError {
    case unableToOpenCHD(String)
    case invalidCHD
    case unsupportedSectorSize
    case sectorReadFailed(UInt32, String)
    case invalidISO9660
    case systemCNFNotFound
    case invalidSystemCNF
    case gameIDNotFound
    
    public var errorDescription: String? {
        switch self {
        case .unableToOpenCHD(let error):
            "Unable to open CHD: \(error)"
        case .invalidCHD:
            "The CHD is invalid."
        case .unsupportedSectorSize:
            "The CHD does not contain a supported CD sector format."
        case .sectorReadFailed(let sector, let error):
            "Unable to read CD sector \(sector): \(error)"
        case .invalidISO9660:
            "The CHD does not contain a valid ISO9660 filesystem."
        case .systemCNFNotFound:
            "SYSTEM.CNF was not found."
        case .invalidSystemCNF:
            "SYSTEM.CNF could not be read."
        case .gameIDNotFound:
            "A PlayStation game ID could not be found in SYSTEM.CNF."
        }
    }
}

public struct ID {
    public let id: String
    public let systemCNF: String
    
    public init(id: String, systemCNF: String) {
        self.id = id
        self.systemCNF = systemCNF
    }
}

public final class CHDReader {
    private struct ISOFile {
        let name: String
        let extent: UInt32
        let size: UInt32
        let isDirectory: Bool
    }
    
    private var reader: UnsafeMutablePointer<chd_reader.CHDReader>?
    private static let sectorSize = 2048
    
    public init(url: URL) throws {
        guard url.isFileURL else { throw IDError.invalidCHD }
        
        reader = url.path.withCString { path in
            chd_reader.chd_reader_open(path)
        }
        
        guard reader != nil else { throw IDError.invalidCHD }
    }
    
    deinit { close() }
    
    public func close() {
        if let reader {
            chd_reader.chd_reader_close(reader)
            self.reader = nil
        }
    }
    
    private var frameSize: Int {
        guard let reader else { return 0 }
        return Int(chd_reader.chd_reader_frame_bytes(reader))
    }
    
    private func readFrame(_ frame: UInt32) throws -> Data {
        guard let reader else { throw IDError.invalidCHD }
        
        let size = frameSize
        guard size > 0 else { throw IDError.unsupportedSectorSize }
        
        var data = Data(count: size)
        
        let success = data.withUnsafeMutableBytes { buffer in
            guard let baseAddress = buffer.baseAddress else { return false }
            return chd_reader.chd_reader_read_frame(
                reader,
                frame,
                baseAddress.assumingMemoryBound(to: UInt8.self)
            ) != 0
        }
        
        guard success else {
            let error = String(cString: chd_reader.chd_reader_error(reader))
            throw IDError.sectorReadFailed(frame, error)
        }
        
        return data
    }
    
    private func readSector(_ sector: UInt32) throws -> Data {
        let frame = try readFrame(sector)
        
        guard frame.count >= 24 + Self.sectorSize else { throw IDError.invalidCHD }
        
        let mode = frame[15]
        
        switch mode {
        case 1:
            return frame.subdata(in: 16 ..< 16 + Self.sectorSize)
            
        case 2:
            return frame.subdata(in: 24 ..< 24 + Self.sectorSize)
            
        default:
            throw IDError.invalidISO9660
        }
    }
    
    private static func littleEndianUInt32(_ data: Data, _ offset: Int) -> UInt32 {
        guard offset + 4 <= data.count else { return 0 }
        return UInt32(data[offset]) |
        (UInt32(data[offset + 1]) << 8) |
        (UInt32(data[offset + 2]) << 16) |
        (UInt32(data[offset + 3]) << 24)
    }
    
    private static func littleEndianUInt16(_ data: Data, _ offset: Int) -> UInt16 {
        guard offset + 2 <= data.count else { return 0 }
        return UInt16(data[offset]) | (UInt16(data[offset + 1]) << 8)
    }
    
    private func parseDirectory(sector: UInt32, size: UInt32) throws -> [ISOFile] {
        var result: [ISOFile] = []
        
        let sectorCount = (Int(size) + Self.sectorSize - 1) / Self.sectorSize
        
        for index in 0..<sectorCount {
            let currentSector = sector + UInt32(index)
            let data = try readSector(currentSector)
            
            var offset = 0
            while offset < data.count {
                let recordLength = Int(data[offset])
                
                if recordLength == 0 { break }
                
                guard offset + recordLength <= data.count, recordLength >= 34 else { break }
                
                let record = data.subdata(in: offset ..< offset + recordLength)
                
                let extent = Self.littleEndianUInt32(record, 2)
                let fileSize = Self.littleEndianUInt32(record, 10)
                let flags = record[25]
                
                let nameLength = Int(record[32])
                guard 33 + nameLength <= record.count else { break }
                
                let nameData = record.subdata(in: 33 ..< 33 + nameLength)
                let name = String(data: nameData, encoding: .ascii) ?? ""
                
                let cleanName = name
                    .split(separator: ";", maxSplits: 1)
                    .first
                    .map(String.init) ?? name
                
                let isDirectory = (flags & 0x02) != 0
                
                if cleanName != "\u{0}" && cleanName != "\u{1}" {
                    result.append(ISOFile(name: cleanName, extent: extent, size: fileSize, isDirectory: isDirectory))
                }
                
                offset += recordLength
            }
        }
        
        return result
    }
    
    private func findSystemCNF() throws -> ISOFile {
        let pvd = try readSector(16)
        
        guard
            pvd.count >= Self.sectorSize,
            pvd[0] == 1,
            String(data: pvd.subdata(in: 1..<6), encoding: .ascii) == "CD001"
                else {
            throw IDError.invalidISO9660
        }
        
        let rootOffset = 156
        guard rootOffset + 34 <= pvd.count else { throw IDError.invalidISO9660 }
        
        let rootExtent = Self.littleEndianUInt32(pvd, rootOffset + 2)
        let rootSize = Self.littleEndianUInt32(pvd, rootOffset + 10)
        
        guard rootExtent != 0, rootSize != 0 else { throw IDError.invalidISO9660 }
        
        let files = try parseDirectory(sector: rootExtent, size: rootSize)
        
        guard let systemCNF = files.first(where: { $0.name.uppercased().hasPrefix("SYSTEM.CNF") }) else {
            throw IDError.systemCNFNotFound
        }
        
        return systemCNF
    }
    
    private func readFile(_ file: ISOFile) throws -> Data {
        guard !file.isDirectory else { throw IDError.invalidSystemCNF }
        
        let fileSize = Int(file.size)
        guard fileSize > 0 else { return Data() }
        
        var result = Data()
        result.reserveCapacity(fileSize)
        
        let sectorCount = (fileSize + Self.sectorSize - 1) / Self.sectorSize
        
        for index in 0..<sectorCount {
            let sector = file.extent + UInt32(index)
            let data = try readSector(sector)
            
            let remaining = fileSize - result.count
            let amount = min(remaining, data.count)
            
            result.append(data.prefix(amount))
            if result.count >= fileSize { break }
        }
        
        return result
    }
    
    private func readSystemCNF() throws -> String {
        let file = try findSystemCNF()
        let data = try readFile(file)
        
        guard let string = String(data: data, encoding: .ascii) else { throw IDError.invalidSystemCNF }
        return string
    }
    
    public func gameIdentifier() throws -> ID {
        let systemCNF = try readSystemCNF()
        
        guard let id = Self.extractGameID(from: systemCNF) else {
            throw IDError.gameIDNotFound
        }
        
        return ID(id: id, systemCNF: systemCNF)
    }
    
    public static func extractGameID(from systemCNF: String) -> String? {
        let pattern = #"(?i)\b([A-Z]{4})[_\-.](\d{3})[_\-.](\d{2})\b"#
        
        guard let regex = try? NSRegularExpression(pattern: pattern) else { return nil }
        
        let range = NSRange(systemCNF.startIndex..., in: systemCNF)
        guard let match = regex.firstMatch(in: systemCNF, range: range) else { return nil }
        
        guard
            let regionRange = Range(match.range(at: 1), in: systemCNF),
            let numberRange = Range(match.range(at: 2), in: systemCNF),
            let revisionRange = Range(match.range(at: 3), in: systemCNF)
                else {
            return nil
        }
        
        let region = String(systemCNF[regionRange]).uppercased()
        let number = String(systemCNF[numberRange])
        let revision = String(systemCNF[revisionRange])
        
        return "\(region)-\(number).\(revision)"
    }
}
