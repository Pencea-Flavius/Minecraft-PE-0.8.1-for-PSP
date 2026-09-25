#ifndef MCPSP_CLIENT_RENDERER_ITEM_ANIM_ICON_H
#define MCPSP_CLIENT_RENDERER_ITEM_ANIM_ICON_H

struct Texture;
class Player;

const Texture* itemAnimIcon(short id, int frame, int* sx, int* sy);

int itemAnimFrame(short id, const Player* holder);

int itemAnimStage(short id, const Player* holder);

#endif
