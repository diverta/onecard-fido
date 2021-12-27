//
//  ToolGPGCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/12/16.
//
#ifndef ToolGPGCommand_h
#define ToolGPGCommand_h

@interface ToolGPGCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (void)setParametersForGeneratePGPKey:(id)sender
        realName:(NSString *)realName mailAddress:(NSString *)mailAddress comment:(NSString *)comment
        passphrase:(NSString *)passphrase exportFolderPath:(NSString *)exportFolderPath;
    - (void)generatePGPKeyWillStart:(id)sender;

@end

#endif /* ToolGPGCommand_h */
