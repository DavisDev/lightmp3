//    LightMP3
//    Copyright (C) 2007,2008 Sakya
//    sakya_tg@yahoo.it
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#include <pspkernel.h>
#include <pspsdk.h>
#include <stdio.h>
#include <stdlib.h>
#include <oslib/oslib.h>

#include "../main.h"
#include "gui_settings.h"
#include "common.h"
#include "languages.h"
#include "settings.h"
#include "skinsettings.h"
#include "menu.h"
#include "system/clock.h"
#include "players/player.h"
#include "../others/audioscrobbler.h"

#define STATUS_CONFIRM_NONE 0
#define STATUS_CONFIRM_SAVE 1
#define STATUS_HELP 2

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Globals:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int settingsRetValue = 0;
static int exitFlagSettings = 0;
char buffer[264] = "";

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get a settings value from menu index:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int getSettingVal(int index, char *value){
    char yesNo[2][10];
    char bCheck[3][30];

    strcpy(yesNo[0], langGetString("NO"));
    strcpy(yesNo[1], langGetString("YES"));

    strcpy(bCheck[0], langGetString("BRIGHTNESS_NO"));
    strcpy(bCheck[1], langGetString("BRIGHTNESS_YES_WARNING"));
    strcpy(bCheck[2], langGetString("BRIGHTNESS_YES"));

    switch(index){
        case 0:
            sprintf(value, "%s", userSettings->skinName);
            break;
        case 1:
            sprintf(value, "%s", userSettings->lang);
            break;
        case 2:
            sprintf(value, "%s", yesNo[userSettings->SCROBBLER]);
            break;
        case 3:
            sprintf(value, "%s", yesNo[userSettings->FADE_OUT]);
            break;
        case 4:
            sprintf(value, "%i", userSettings->MUTED_VOLUME);
            break;
        case 5:
            sprintf(value, "%s", bCheck[userSettings->BRIGHTNESS_CHECK]);
            break;
        case 6:
            sprintf(value, "%s", yesNo[userSettings->MP3_ME]);
            break;
        case 7:
            sprintf(buffer, "BOOST_%s", userSettings->BOOST);
            sprintf(value, "%s", langGetString(buffer));
            break;
        case 8:
            sprintf(value, "%s", yesNo[userSettings->CLOCK_AUTO]);
            break;
        case 9:
            sprintf(value, "%i", userSettings->CLOCK_GUI);
            break;
        case 10:
            sprintf(value, "%i", userSettings->CLOCK_MP3);
            break;
        case 11:
            sprintf(value, "%i", userSettings->CLOCK_MP3ME);
            break;
        case 12:
            sprintf(value, "%i", userSettings->CLOCK_OGG);
            break;
        case 13:
            sprintf(value, "%i", userSettings->CLOCK_FLAC);
            break;
        case 14:
            sprintf(value, "%i", userSettings->CLOCK_AA3);
            break;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Init settings menu:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int initSettingsMenu(){
    //Build menu:
    commonMenu.first = 0;
    commonMenu.selected = 0;
    skinGetPosition("POS_SETTINGS", tempPos);
    commonMenu.yPos = tempPos[1];
    commonMenu.xPos = tempPos[0];
    commonMenu.fastScrolling = 0;
    commonMenu.align = ALIGN_LEFT;
    sprintf(buffer, "%s/menubkg.png", userSettings->skinImagesPath);
    commonMenu.background = oslLoadImageFilePNG(buffer, OSL_IN_RAM | OSL_SWIZZLED, OSL_PF_8888);
    if (!commonMenu.background)
        errorLoadImage(buffer);
    commonMenu.highlight = commonMenuHighlight;
    commonMenu.width = commonMenu.background->sizeX;
    commonMenu.height = commonMenu.background->sizeY;
    commonMenu.interline = 1;
    commonMenu.maxNumberVisible = commonMenu.background->sizeY / (fontNormal->charHeight + commonMenu.interline);
    commonMenu.cancelFunction = NULL;

    //Values menu:
    commonSubMenu.yPos = tempPos[1];
    commonSubMenu.xPos = tempPos[0] + 350;
    commonSubMenu.align = ALIGN_RIGHT;
    commonSubMenu.fastScrolling = 0;
    commonSubMenu.background = NULL;
    commonSubMenu.highlight = NULL;
    commonSubMenu.width = commonMenu.width - 350;
    commonSubMenu.height = commonMenu.height;
    commonSubMenu.interline = commonMenu.interline;
    commonSubMenu.maxNumberVisible = commonMenu.maxNumberVisible;
    commonSubMenu.cancelFunction = NULL;
    return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Build settings menu:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int buildSettingsMenu(struct menuElements *menu, struct menuElements *values){
    int i = 0;
    char name[100] = "";
    char settingVal[10] = "";
    struct menuElement tMenuEl;

    menu->numberOfElements = 15;

    values->first = menu->first;
    values->selected = menu->selected;
    values->numberOfElements = menu->numberOfElements;
    for (i=0; i<menu->numberOfElements; i++){
        sprintf(name, "SETTINGS_%2.2i", i + 1);
        strcpy(tMenuEl.text, langGetString(name));
        tMenuEl.triggerFunction = NULL;
        menu->elements[i] = tMenuEl;
        getSettingVal(i, settingVal);
        strcpy(tMenuEl.text, settingVal);
        tMenuEl.triggerFunction = NULL;
        values->elements[i] = tMenuEl;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Change a setting value from menu index:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int changeSettingVal(int index, int delta){
    int current = 0;
    int i = 0;

    switch(index){
        case 0:
            for (i=0; i<skinsCount; i++){
                if (!strcmp(skinsList[i], userSettings->skinName)){
                    current = i;
                    break;
                }
            }
            if (current + delta < skinsCount && current + delta >= 0){
                drawWait(langGetString("LOADING_SKIN_TITLE"), langGetString("LOADING_SKIN"));
                oslEndDrawing();
                oslEndFrame();
                oslSyncFrame();
                current += delta;
                strcpy(userSettings->skinName, skinsList[current]);
                sprintf(buffer, "%sskins/%s/skin.cfg", userSettings->ebootPath, userSettings->skinName);
                skinLoad(buffer);
                sprintf(userSettings->skinImagesPath, "%sskins/%s/images", userSettings->ebootPath, userSettings->skinName);
                unLoadCommonGraphics();
                loadCommonGraphics();
                initSettingsMenu();
                oslStartDrawing();
            }
            break;
        case 1:
            for (i=0; i<languagesCount; i++){
                if (!strcmp(languagesList[i], userSettings->lang)){
                    current = i;
                    break;
                }
            }
            if (current + delta < languagesCount && current + delta >= 0){
                current += delta;
                strcpy(userSettings->lang, languagesList[current]);
            }
            sprintf(buffer, "%slanguages/%s/lang.txt", userSettings->ebootPath, userSettings->lang);
            if (langLoad(buffer)){
                debugMessageBox("Error loading language file.");
            	oslQuit ();
                return 0;
            }
            buildSettingsMenu(&commonMenu, &commonSubMenu);
            break;
        case 2:
            userSettings->SCROBBLER = !userSettings->SCROBBLER;
            //AudioScrobbler:
            if (userSettings->SCROBBLER){
                sprintf(buffer, "%s%s", userSettings->ebootPath, ".scrobbler.log");
                SCROBBLER_init(buffer);
            }
            break;
        case 3:
            userSettings->FADE_OUT = !userSettings->FADE_OUT;
            break;
        case 4:
            if (userSettings->MUTED_VOLUME + delta < 8000 && userSettings->MUTED_VOLUME + delta >= 0)
                userSettings->MUTED_VOLUME += delta;
            break;
        case 5:
            if (++userSettings->BRIGHTNESS_CHECK > 2)
                userSettings->BRIGHTNESS_CHECK = 0;
            break;
        case 6:
            userSettings->MP3_ME = !userSettings->MP3_ME;
            break;
        case 7:
            if (!strcmp(userSettings->BOOST, "NEW"))
                strcpy(userSettings->BOOST, "OLD");
            else
                strcpy(userSettings->BOOST, "NEW");
            break;
        case 8:
            userSettings->CLOCK_AUTO = !userSettings->CLOCK_AUTO;
            break;
        case 9:
            if (userSettings->CLOCK_GUI + delta <= 222 && userSettings->CLOCK_GUI + delta >= getMinCPUClock())
                userSettings->CLOCK_GUI += delta;
            break;
        case 10:
            if (userSettings->CLOCK_MP3 + delta <= 222 && userSettings->CLOCK_MP3 + delta >= getMinCPUClock())
                userSettings->CLOCK_MP3 += delta;
                MP3_defaultCPUClock = userSettings->CLOCK_MP3;
            break;
        case 11:
            if (userSettings->CLOCK_MP3ME + delta <= 222 && userSettings->CLOCK_MP3ME + delta >= getMinCPUClock())
                userSettings->CLOCK_MP3ME += delta;
                MP3ME_defaultCPUClock = userSettings->CLOCK_MP3ME;
            break;
        case 12:
            if (userSettings->CLOCK_OGG + delta <= 222 && userSettings->CLOCK_OGG + delta >= getMinCPUClock())
                userSettings->CLOCK_OGG += delta;
                OGG_defaultCPUClock = userSettings->CLOCK_OGG;
            break;
        case 13:
            if (userSettings->CLOCK_FLAC + delta <= 222 && userSettings->CLOCK_FLAC + delta >= getMinCPUClock())
                userSettings->CLOCK_FLAC += delta;
                FLAC_defaultCPUClock = userSettings->CLOCK_FLAC;
            break;
        case 14:
            if (userSettings->CLOCK_AA3 + delta <= 222 && userSettings->CLOCK_AA3 + delta >= getMinCPUClock())
                userSettings->CLOCK_AA3 += delta;
                AA3ME_defaultCPUClock = userSettings->CLOCK_AA3;
            break;
    }
    return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Settings:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int gui_settings(){
    int confirmStatus = STATUS_CONFIRM_NONE;

    initSettingsMenu();
    buildSettingsMenu(&commonMenu, &commonSubMenu);

    exitFlagSettings = 0;
    while(!osl_quit && !exitFlagSettings){
        oslStartDrawing();
        drawCommonGraphics();
        drawButtonBar(MODE_SETTINGS);
        drawMenu(&commonMenu);
        drawMenu(&commonSubMenu);

        switch (confirmStatus){
            case STATUS_CONFIRM_SAVE:
                drawConfirm(langGetString("CONFIRM_SAVE_SETTINGS_TITLE"), langGetString("CONFIRM_SAVE_SETTINGS"));
                break;
            case STATUS_HELP:
                drawHelp("SETTINGS");
                break;
        }

        oslReadKeys();
        if (!osl_pad.pressed.hold){
            if (confirmStatus == STATUS_CONFIRM_SAVE){
                if(osl_pad.released.cross){
                    SETTINGS_save(userSettings);
                    confirmStatus = STATUS_CONFIRM_NONE;
                }else if(osl_pad.pressed.circle){
                    confirmStatus = STATUS_CONFIRM_NONE;
                }
            }else if (confirmStatus == STATUS_HELP){
                if (osl_pad.released.cross || osl_pad.released.circle)
                    confirmStatus = STATUS_CONFIRM_NONE;
            }else{
                processMenuKeys(&commonMenu);
                commonSubMenu.selected = commonMenu.selected;
                commonSubMenu.first = commonMenu.first;

                if (osl_pad.held.L && osl_pad.held.R){
                    confirmStatus = STATUS_HELP;
                }else if (osl_pad.pressed.right || osl_pad.analogX > ANALOG_SENS){
                    changeSettingVal(commonMenu.selected, +1);
                    buildSettingsMenu(&commonMenu, &commonSubMenu);
                    sceKernelDelayThread(100000);
                }else if (osl_pad.pressed.left || osl_pad.analogX < -ANALOG_SENS){
                    changeSettingVal(commonMenu.selected, -1);
                    buildSettingsMenu(&commonMenu, &commonSubMenu);
                    sceKernelDelayThread(100000);
                }else if(osl_pad.released.start){
                    confirmStatus = STATUS_CONFIRM_SAVE;
                }else if(osl_pad.released.R){
                    settingsRetValue = nextAppMode(MODE_SETTINGS);
                    exitFlagSettings = 1;
                }else if(osl_pad.released.L){
                    settingsRetValue = previousAppMode(MODE_SETTINGS);
                    exitFlagSettings = 1;
                }
            }
        }
    	oslEndDrawing();
        oslEndFrame();
    	oslSyncFrame();
    }
    //unLoad images:
    clearMenu(&commonMenu);
    clearMenu(&commonSubMenu);

    return settingsRetValue;
}