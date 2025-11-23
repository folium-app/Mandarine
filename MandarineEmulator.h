//
//  MandarineEmulator.h
//  Mandarine
//
//  Created by Jarrod Norwell on 17/11/2025.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface MandarineEmulator : NSObject
@property (nonatomic, strong, nullable) void (^bgr555) (void*, NSInteger, NSInteger, NSInteger, NSInteger);
@property (nonatomic, strong, nullable) void (^rgb888) (uint16_t*, NSInteger, NSInteger, NSInteger, NSInteger);

+(MandarineEmulator *) sharedInstance NS_SWIFT_NAME(shared());

-(void) insertCartridge:(NSURL *)url NS_SWIFT_NAME(insert(_:));

-(void) start;
-(void) pause:(BOOL)pause;
-(BOOL) isPaused;

-(void) input:(NSInteger)slot button:(NSString *)button pressed:(BOOL)pressed;
-(void) drag:(NSInteger)slot stick:(NSString *)stick value:(int16_t)value;

-(NSString *) id:(NSURL *)url NS_SWIFT_NAME(id(from:));
@end

NS_ASSUME_NONNULL_END
