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
#include <psppower.h>
#include <psprtc.h>
#include <stdio.h>
#include <oslib/oslib.h>
#include "common.h"
#include "skinsettings.h"
#include "main.h"
#include "menu.h"

OSL_FONT *fontMenuNormal;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Globals:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// initMenu: loads fonts
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int initMenu(){
    char buffer[264];
    //sprintf(buffer, "%s/fontNormal.oft", userSettings->skinImagesPath);
    sprintf(buffer, "flash0:/font/ltn0.pgf");
    //sprintf(buffer, "%s/fontNormal.pgf", userSettings->skinImagesPath);
    fontMenuNormal = oslLoadFontFile(buffer);
    if (!fontMenuNormal)
        errorLoadImage(buffer);
    oslIntraFontSetStyle(fontMenuNormal, 0.5f, 0xFFFFFFFF, 0xFF000000, INTRAFONT_ALIGN_LEFT);
    fontMenuNormal->charHeight -= 2;

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// clearMenu: clear menu data:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int clearMenu(struct menuElements *menu){
    int i;

    menu->selected = -1;
    menu->first = 0;
    menu->maxNumberVisible = 0;
    menu->yPos = 0;
    menu->xPos = 0;
    menu->width = 0;
    menu->height =  0;
    if (menu->background)
        oslDeleteImage(menu->background);
    if (menu->highlight && menu->highlight != commonMenuHighlight)
        oslDeleteImage(menu->highlight);
    for (i=0; i < menu->numberOfElements; i++){
        strcpy(menu->elements[i].text, "");
        menu->elements[i].triggerFunction = NULL;
    }
    menu->align = ALIGN_LEFT;
    menu->numberOfElements = 0;
    menu->interline = 0;
    menu->cancelFunction = NULL;
    menu->dataFeedFunction = NULL;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// drawMenu: draws a menu on video
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int drawMenu(struct menuElements *menu){
    int i = 0;
    int count = 0;
    int xPos = 0;
    int yPos = 0;

    int startY = menu->yPos + (float)(menu->height -  menu->maxNumberVisible * (fontMenuNormal->charHeight + menu->interline)) / 2.0;
    if (menu->background != NULL){
        oslDrawImageXY(menu->background, menu->xPos, menu->yPos);
        if (menu->highlight != NULL)
            menu->highlight->stretchX = menu->background->sizeX;
    }

    oslSetFont(fontMenuNormal);
    for (i=menu->first; i<menu->first + menu->maxNumberVisible; i++){
        if (i >= menu->numberOfElements)
            break;

        yPos = startY + fontMenuNormal->charHeight * count + menu->interline * count;
        if (i == menu->selected){
            if (menu->highlight != NULL)
                oslDrawImageXY(menu->highlight, menu->xPos, yPos);
			skinGetColor("RGBA_MENU_SELECTED_TEXT", tempColor);
            skinGetColor("RGBA_MENU_SELECTED_TEXT_SHADOW", tempColorShadow);
            oslIntraFontSetStyle(fontMenuNormal, 0.5f, RGBA(tempColor[0], tempColor[1], tempColor[2], tempColor[3]), RGBA(tempColorShadow[0], tempColorShadow[1], tempColorShadow[2], tempColorShadow[3]), INTRAFONT_ALIGN_LEFT);
        }else{
            skinGetColor("RGBA_MENU_TEXT", tempColor);
            skinGetColor("RGBA_MENU_TEXT_SHADOW", tempColorShadow);
            oslIntraFontSetStyle(fontMenuNormal, 0.5f, RGBA(tempColor[0], tempColor[1], tempColor[2], tempColor[3]), RGBA(tempColorShadow[0], tempColorShadow[1], tempColorShadow[2], tempColorShadow[3]), INTRAFONT_ALIGN_LEFT);
        }

        if (menu->dataFeedFunction != NULL)
            menu->dataFeedFunction(i, &menu->elements[i]);

        if (menu->align == ALIGN_LEFT)
            xPos = menu->xPos + 4;
        else if (menu->align == ALIGN_RIGHT)
            xPos = menu->xPos + menu->width - oslGetStringWidth(menu->elements[i].text) - 4;
        else if (menu->align == ALIGN_CENTER)
            xPos = menu->xPos + (menu->width - oslGetStringWidth(menu->elements[i].text)) / 2;

        oslDrawString(xPos, yPos, menu->elements[i].text);
        count++;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// processMenuKeys: process the button pressed
// NOTE: the oslReadKeys(); must be done in the calling function.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int processMenuKeys(struct menuElements *menu){
    struct menuElement selected;

    if (menu->numberOfElements && osl_pad.pressed.down){
        if (menu->selected < menu->numberOfElements - 1){
            menu->selected++;
            if (menu->selected >= menu->first + menu->maxNumberVisible)
                menu->first++;
        }else{
            menu->selected = 0;
            menu->first = 0;
        }
    }else if (menu->numberOfElements && osl_pad.pressed.up){
        if (menu->selected > 0){
            menu->selected--;
            if (menu->selected < menu->first)
                menu->first--;
        }else{
            menu->selected = menu->numberOfElements - 1;
            menu->first = menu->selected - menu->maxNumberVisible + 1;
            if (menu->first < 0)
                menu->first = 0;
        }
    }else if (menu->numberOfElements && menu->fastScrolling && osl_pad.pressed.right){
    	if (menu->first + menu->maxNumberVisible < menu->numberOfElements){
    		menu->first = menu->first + menu->maxNumberVisible;
    		menu->selected += menu->maxNumberVisible;
    		if (menu->selected > menu->numberOfElements - 1){
    			menu->selected = menu->numberOfElements - 1;
    		}
    	} else {
    		menu->selected = menu->numberOfElements - 1;
    	}
    }else if (menu->numberOfElements && menu->fastScrolling && osl_pad.pressed.left){
    	if (menu->first - menu->maxNumberVisible >= 0){
    		menu->first = menu->first - menu->maxNumberVisible;
    		menu->selected -= menu->maxNumberVisible;
    	} else {
    		menu->first = 0;
    		menu->selected = 0;
    	}
    }else if (menu->numberOfElements && osl_pad.pressed.cross){
        selected = menu->elements[menu->selected];
        if (selected.triggerFunction != NULL)
            selected.triggerFunction();
    }else if (osl_pad.pressed.circle){
        if (menu->cancelFunction != NULL)
            menu->cancelFunction();
    }
    return 0;
}