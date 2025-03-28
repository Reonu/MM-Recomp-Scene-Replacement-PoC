#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "PR/gbi.h"

#include "mm_test_scene.h"
#include "minecraft_spider.h"
#include "minecraft_spider_anims.h"
#include "minecraft_chest_skeleton.h"
#include "minecraft_chest_skeleton_anim.h"
#include "wrench.h"
#include "eztr_api.h"
#include "message_data_fmt_nes.h"

#define REPLACED_SCENE SCENE_KINSTA1
//#define REPLACED_SCENE SCENE_INSIDETOWER

// Populate this when you add more rooms
void* custom_scene_rooms[] = {
    mm_test_room_0_header00,
};

// Dummy symbols to let the rooms compile
u8 _mm_test_room_0SegmentRomStart[1];
u8 _mm_test_room_0SegmentRomEnd[1];

RECOMP_HOOK("Play_InitScene") void on_init_scene(PlayState* play, s32 spawn) {
    recomp_printf("Spawning into scene %d (spawn %d)\n", play->sceneId, spawn);

    if (play->sceneId == REPLACED_SCENE) {
        // Replace the sceneSegment with our custom one. No need to free the original memory since it's part of an arena
        // and will get freed automatically when resetting between scenes.
        play->sceneSegment = mm_test_scene_header00;
    }
}

RECOMP_HOOK("Room_RequestNewRoom") void on_room_request(PlayState* play, RoomContext* roomCtx, s32 index) {
    if (play->sceneId == REPLACED_SCENE && roomCtx->status == 0) {
        roomCtx->prevRoom = roomCtx->curRoom;
        roomCtx->curRoom.num = index;
        roomCtx->curRoom.segment = NULL;
        roomCtx->status = 1;
        roomCtx->roomRequestAddr = custom_scene_rooms[index];
        roomCtx->activeBufPage ^= 1;

        // Record that a room load request has to be sent after the request function returns.
        osCreateMesgQueue(&roomCtx->loadQueue, roomCtx->loadMsg, ARRAY_COUNT(roomCtx->loadMsg));
        osSendMesg(&roomCtx->loadQueue, NULL, OS_MESG_NOBLOCK);
    }
}

RECOMP_HOOK("BgCheck_GetSpecialSceneMaxObjects") void set_col_memsize(PlayState* play, s32* maxNodes, s32* maxPolygons, s32* maxVertices) {
    if (play->sceneId == REPLACED_SCENE) {
        play->colCtx.memSize = 0x23000 * 2; // increase this if you need more surface nodes
    }
}

extern SkeletonHeader object_st_Skel_005298;
extern AnimationHeader object_st_Anim_000304;
RECOMP_HOOK("EnSw_Init") void on_EnSw_Init(Actor* thisx, PlayState* play) {
    *(SkeletonHeader*)Lib_SegmentedToVirtual(&object_st_Skel_005298) = minecraft_spider;
    *(AnimationHeader*)Lib_SegmentedToVirtual(&object_st_Anim_000304) = minecraft_spiderMinecraft_spider_normalAnim;
}

RECOMP_PATCH s32 EnSw_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* thisx) {
    return 0;
}

extern Gfx minecraft_chest_base[];
extern Gfx minecraft_chest_lid[];
RECOMP_PATCH void EnBox_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, Actor* thisx, Gfx** gfx) {

}

extern SkeletonHeader gBoxChestSkel;
extern AnimationHeader gBoxBigChestOpenChildAnim;
extern AnimationHeader gBoxChestOpenAnim;

RECOMP_HOOK ("EnBox_Init") void on_EnBox_Init(Actor* thisx, PlayState* play) {
    *(SkeletonHeader*)Lib_SegmentedToVirtual(&gBoxChestSkel) = minecraft_chest_skeleton;
    *(AnimationHeader*)Lib_SegmentedToVirtual(&gBoxBigChestOpenChildAnim) = minecraft_chest_skeletonRest_poseAnim;
    *(AnimationHeader*)Lib_SegmentedToVirtual(&gBoxChestOpenAnim) = minecraft_chest_skeletonRest_poseAnim;
}

RECOMP_HOOK ("EnBox_Draw") void on_EnBox_Draw(Actor* thisx, PlayState* play) {
    thisx->scale.x = 0.008f;
    thisx->scale.y = 0.008f;
    thisx->scale.z = 0.008f;
}
RECOMP_HOOK("DemoTreLgt_Draw") void on_DemoTreeLgt_Draw(Actor* thisx, PlayState* play) {
    thisx->scale.x = 0.012f;
    thisx->scale.z = 0.0109f;
}

RECOMP_PATCH void GetItem_DrawOpa0Xlu1(PlayState* play, s16 drawId) {
    s32 pad;
    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL25_Opa(play->state.gfxCtx);

    MATRIX_FINALIZE_AND_LOAD(POLY_OPA_DISP++, play->state.gfxCtx);
    gSPDisplayList(POLY_OPA_DISP++, wrench);

    CLOSE_DISPS(play->state.gfxCtx);    
}

EZTR_ON_INIT void replace_msgs() {
    EZTR_Basic_ReplaceText(
        0x0059,
        EZTR_STANDARD_TEXT_BOX_I,
        1,
        EZTR_ICON_RAZOR_SWORD,
        EZTR_NO_VALUE,
        EZTR_NO_VALUE,
        EZTR_NO_VALUE,
        true,
        "You got|01 Modding Support|00! |BF",

        NULL
    );
}