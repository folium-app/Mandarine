//
//  MandarineEmulator.h
//  Mandarine
//
//  Created by Jarrod Norwell on 17/11/2025.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface MandarineEmulator : NSObject
@property (nonatomic, strong, nullable) void (^audioCallback) (uint16_t*, NSInteger);
@property (nonatomic, strong, nullable) void (^videoCallback) (void*, NSInteger, NSInteger, NSInteger, NSInteger);
@property (nonatomic, strong, nullable) void (^secondaryVideoCallback) (uint16_t*, NSInteger, NSInteger, NSInteger, NSInteger);

+(MandarineEmulator *) sharedInstance NS_SWIFT_NAME(shared());

-(void) insertCartridge:(NSURL *)url NS_SWIFT_NAME(insert(cartridge:));

-(void) pause;
-(void) start;
-(void) stop;
-(void) unpause;

-(BOOL) isPaused NS_SWIFT_NAME(paused());
-(BOOL) isRunning NS_SWIFT_NAME(running());

-(void) press:(NSString *)button;
-(void) release:(NSString *)button;

-(void) load:(NSURL *)url NS_SWIFT_NAME(load(state:));
-(void) save:(NSURL *)url NS_SWIFT_NAME(save(state:));

-(NSString *) identifier:(NSURL *)url NS_SWIFT_NAME(identifier(cartridge:));
@end

NS_ASSUME_NONNULL_END
