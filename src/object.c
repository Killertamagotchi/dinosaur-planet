#include "common.h"

#include "common.h"
#include "variables.h"
#include "sys/gfx/model.h"

extern void **gLoadedObjDefs;
extern void *D_800B1918;
extern void *D_800B18E4;
extern int gObjIndexCount; //count of OBJINDEX.BIN entries
extern int gNumObjectsTabEntries;
extern ObjData *gLoadedObjData;
extern u8 *gObjRefCount; //pObjectRefCount
extern int gNumTablesTabEntries;
extern TActor **gObjList; //global object list

enum FILE_ID {
    FILE_TABLES_BIN   = 0x16,
    FILE_TABLES_TAB   = 0x17,
    FILE_BITTABLE     = 0x37,
    FILE_OBJECTS_TAB  = 0x41,
    FILE_OBJINDEX_BIN = 0x43
};

extern void *gFile_TABLES_BIN;
extern s32  *gFile_TABLES_TAB;
extern s32  *gFile_OBJECTS_TAB;
extern s16  *gFile_OBJINDEX;

int get_file_size(int file);
void queue_alloc_load_file(void **dest, s32 fileId);
void queue_load_file_to_ptr(void **dest, s32 fileId);
void alloc_some_object_arrays(void); //related to objects
void func_80020D34(void);

void copy_obj_position_mirrors(TActor *obj, ActorUnk0x1a *param2, s32 param3);

void func_80046320(s16 param1, TActor *actor);
void func_80023A00(TActor *actor, s8 param2); // sets actor->unk0xae = param2
void func_800222AC(TActor *actor);

extern char D_80099600[]; // "objects/objects.c"
extern s32 gNumObjs;

void init_objects(void) {
    int i;

    //allocate some buffers
    gLoadedObjDefs = malloc(0x2D0, ALLOC_TAG_OBJECTS_COL, NULL);
    D_800B1918     = malloc(0x60, ALLOC_TAG_OBJECTS_COL,  NULL);
    D_800B18E4     = malloc(0x10, ALLOC_TAG_OBJECTS_COL,  NULL);

    //load OBJINDEX.BIN and count number of entries
    queue_alloc_load_file((void **) (&gFile_OBJINDEX), FILE_OBJINDEX_BIN);
    gObjIndexCount = (get_file_size(FILE_OBJINDEX_BIN) >> 1) - 1;
    while(!gFile_OBJINDEX[gObjIndexCount]) gObjIndexCount--;

    //load OBJECTS.TAB and count number of entries
    queue_alloc_load_file((void **)&gFile_OBJECTS_TAB, FILE_OBJECTS_TAB);
    gNumObjectsTabEntries = 0;
    while(gFile_OBJECTS_TAB[gNumObjectsTabEntries] != -1) gNumObjectsTabEntries++;
    gNumObjectsTabEntries--;

    //init ref count and pointers
    gLoadedObjData = malloc(gNumObjectsTabEntries * 4, ALLOC_TAG_OBJECTS_COL, NULL);
    gObjRefCount   = malloc(gNumObjectsTabEntries,     ALLOC_TAG_OBJECTS_COL, NULL);
    for(i = 0; i < gNumObjectsTabEntries; i++) gObjRefCount[i] = 0; //why not memset?

    //load TABLES.BIN and TABLES.TAB and count number of entries
    queue_alloc_load_file((void **) (&gFile_TABLES_BIN), FILE_TABLES_BIN);
    queue_alloc_load_file((void **) (&gFile_TABLES_TAB), FILE_TABLES_TAB);
    gNumTablesTabEntries = 0;
    while(gFile_TABLES_TAB[gNumTablesTabEntries] != -1) gNumTablesTabEntries++;

    //allocate global object list and some other buffers
    gObjList = malloc(0x2D0, ALLOC_TAG_OBJECTS_COL, NULL);
    alloc_some_object_arrays();
    func_80020D34();
}

#pragma GLOBAL_ASM("asm/nonmatchings/object/update_objects.s")
#if 0
extern char D_800994E0;

void func_update_objects(void) {
    s16 size;
    TActor *obj2;
    void *temp_s0_2;
    void *temp_s0_3;
    void *temp_s0_4;
    void *temp_s0_5;
    void *temp_s0_6;
    TActor *child;
    void *temp_t4;
    HitState *hitState;
    TActor *player;
    void *temp_v0_3;
    TActor *phi_s0;
    TActor *obj3;
    void *phi_s0_3;
    TActor *obj;
    void *phi_s0_4;
    void *phi_s0_5;
    void *phi_s0_6;

    size = gObjList->size;
    func_80058FE8();
    update_obj_models();
    update_obj_hitboxes(gNumObjs);

    obj = gObjList->obj;
    while(obj != NULL && obj->priority == 0x64) {
        update_object(obj);
        obj = obj + size;
    }
    while(obj != NULL && obj->data->flags & OBJDATA_FLAG44_HasChildren) {
        update_object(obj);
        obj->mtxIdx = func_80004258(obj);
        obj = obj + size;
    }
    func_80025E58();

    while(obj) {
        hitState = obj->hitState;
        if(hitState != 0) {
            if ((hitState->flags_0x5A != HITSTATE_FLAGS62_DontUpdate)
            || ((hitState->flags_0x58 & HITSTATE_FLAG60_Disabled) == 0)) { //disabled
                update_object(obj);
            }
        } else {
            update_object(obj);
        }
        obj = obj + size;
    }
    
    player = get_player();
    if(player) {
        child = player->children[0];
        if(child != 0) {
            update_object(player->children[0]);
        }
    }

    obj_do_hit_detection(gNumObjs);

    obj = gObjList->obj;
    while(obj) {
        func_8002272C(phi_s0_6);
        obj = obj + size;
    }

    player = get_player();
    if (player != 0) {
        child = player->children;
        if (child != 0) {
            child->ptr0x30 = player->ptr0x30;
            func_8002272C(player->children);
        }
    }

    (*gDLL_waterfx)->func[0].withOneArg(delayByte);
    (*gDLL_projgfx)->func[2].withTwoArgs(delayByte, 0);
    (*gDLL_modgfx)->func[2].withThreeArgs(0, 0, 0);
    (*gDLL_expgfx)->func[2].withFourArgs(0, delayByte, 0, 0);
    func_8002B6EC();
    (*gDLL_ANIM)->func[9].asVoid();
    (*gDLL_ANIM)->func[5].asVoid();
    (*gDLL_Camera)->func[1].withOneArg(delayByte);
    write_cFile_label_pointers(&D_800994E0, 0x169);
}
#endif

void doNothing_80020A40(void) {}

void update_obj_models() {
    int i;
    int j;
    int k;
    TActor *actor;
    ModelInstance *modelInst;
    ActorUnk0xc0_0xb8 *unk1;

    for (i = 0; i < gNumObjs; i++) {
        actor = gObjList[i];

        for (j = 0; j < 2; j++) {
            if (j != 0) {
                actor = actor->linkedActor2;
            }

            if (actor == NULL) {
                continue;
            }

            for (k = 0; k < actor->data->modLinesIdx; k++) {
                modelInst = actor->modelInsts[k];

                if (modelInst != NULL) {
                    modelInst->unk_0x34 &= ~0x8;

                    if (modelInst->model->unk_0x1c != NULL) {
                        unk1 = actor->unk0xc0 != NULL ? actor->unk0xc0->unk_0xb8 : NULL;

                        if (actor->unk0xc0 == NULL || (unk1 != NULL && unk1->unk_0x62 == 0)) {
                            func_8001B084(modelInst, delayFloat);
                        }
                    }
                }
            }
        }
    }
}

extern int D_800B1914;
void func_80020BB8() {
    int i;
    for(i = 0; i < D_800B1914; i++) {
        func_80022F94(gLoadedObjDefs[i], 0); //possibly some type of free?
        gLoadedObjDefs[i] = 0;
    }
    D_800B1914 = 0;
}

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80020C48.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80020D34.s")

extern s16 D_800B18E0;
void func_80020D90(void) { D_800B18E0 = 0; }

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80020DA0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80020EE4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_800210DC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/get_world_actors.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80021178.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_800211B4.s")

s32 get_num_objects(void) { return gNumObjs; }

s32 ret0_800212E8(void) { return 0; }

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_800212F4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_800213A0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_800213EC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/objSetupObjectActual.s")

void func_80021A84(TActor *actor, u32 someFlags) {
    if (actor->linkedActor != NULL) {
        transform_point_by_actor(
            actor->srt.transl.x, actor->srt.transl.y, actor->srt.transl.z,
            &actor->positionMirror.x, &actor->positionMirror.y, &actor->positionMirror.z,
            actor->linkedActor
        );
    } else {
        actor->positionMirror.x = actor->srt.transl.x;
        actor->positionMirror.y = actor->srt.transl.y;
        actor->positionMirror.z = actor->srt.transl.z;
    }

    actor->positionMirror2.x = actor->srt.transl.x;
    actor->positionMirror2.y = actor->srt.transl.y;
    actor->positionMirror2.z = actor->srt.transl.z;

    actor->positionMirror3.x = actor->positionMirror.x;
    actor->positionMirror3.y = actor->positionMirror.y;
    actor->positionMirror3.z = actor->positionMirror.z;

    copy_obj_position_mirrors(actor, actor->unk0x4c, 0);

    if (actor->objhitInfo != NULL) {
        actor->objhitInfo->unk_0x10.x = actor->srt.transl.x;
        actor->objhitInfo->unk_0x10.y = actor->srt.transl.y;
        actor->objhitInfo->unk_0x10.z = actor->srt.transl.z;

        actor->objhitInfo->unk_0x20.x = actor->srt.transl.x;
        actor->objhitInfo->unk_0x20.y = actor->srt.transl.y;
        actor->objhitInfo->unk_0x20.z = actor->srt.transl.z;
    }

    if (actor->data->unka0 > -1) {
        func_80046320(actor->data->unka0, actor);
    }

    update_pi_manager_array(0, -1);

    if (actor->data->flags & OBJDATA_FLAG44_HasChildren) {
        add_object_to_array(actor, 7);

        if (actor->unk0xae != 90) {
            func_80023A00(actor, 90);
        }
    } else {
        if (actor->unk0xae == 0) {
            func_80023A00(actor, 80);
        }
    }

    if (someFlags & 1) {
        actor->unk0xb0 |= 0x10;
        gObjList[gNumObjs] = actor; // TODO: gObjList is typed wrong?
        gNumObjs += 1;

        func_800222AC(actor);
    }

    if (actor->data->numSeqs >= 1) {
        add_object_to_array(actor, 9);
    }

    if (actor->data->flags & OBJDATA_FLAG44_HaveModels) {
        func_80020D90();
    }

    if (actor->data->flags & OBJDATA_FLAG44_DifferentLightColor) {
        add_object_to_array(actor, 56);
    }

    write_c_file_label_pointers(D_80099600, 0x477);
}

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80021CC0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80021E74.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80022150.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80022200.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80022274.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_800222AC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80022338.s")

void copy_obj_position_mirrors(TActor *obj, ActorUnk0x1a *param2, s32 param3)
{
    DLLInst_Unknown *dll;
    obj->objId = obj->data->objId;
    dll = obj->dll;
    if(1) {
        if(dll != NULL) {
            obj->dll->exports->func[0].withThreeArgs((s32)obj, (s32)param2, param3);
        }
    }

    obj->positionMirror2.x = obj->srt.transl.x;
    obj->positionMirror3.x = obj->srt.transl.x;
    obj->positionMirror2.y = obj->srt.transl.y;
    obj->positionMirror3.y = obj->srt.transl.y;
    obj->positionMirror2.z = obj->srt.transl.z;
    obj->positionMirror3.z = obj->srt.transl.z;
}

#pragma GLOBAL_ASM("asm/nonmatchings/object/update_object.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_8002272C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_800227AC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80022828.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80022868.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_800228D0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_8002298C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_800229E8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80022AA4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80022C68.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80022D00.s")

void doNothing_80022DD8(s32 a0, s32 a1, s32 a2) { }

s32 func_80022DEC(void) { return gObjIndexCount; }

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80022DFC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80022E3C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80022EC0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80022F94.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_800233F4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80023464.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80023628.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80023894.s")

TActor *get_player(void) {
    TActor **obj;
    s32 idx;
    obj = TActor_getter(0, &idx);
    if(idx) {} else {}; //wat
    if(idx) return *obj;
    else return NULL;
}

TActor *func_8002394C() {
    TActor **actorList;
    s32 count;

    actorList = TActor_getter(1, &count);

    if (count) {}

    if (count != 0) {
        return actorList[0];
    } else {
        return NULL;
    }
}

void func_80023984(s8 *arg) { arg[0xAC] = -1; }

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80023994.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_800239C0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80023A00.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80023A18.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80023A78.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80023B34.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80023B60.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80023BF8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80023C6C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80023CD8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80023D08.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80023D30.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_800240BC.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80024108.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_8002493C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80024D74.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80024DD0.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80024E2C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80024E50.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_800250F4.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80025140.s")

#if 1
#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80025540.s")
#else
//in retail SFA 1.0 I believe this is at 8002ed18
//but it's different because SFA doesn't use Model Instances
//or, this might be a completely different function...
void func_80025540(TActor *obj, s32 a1, s32 a2)
{
    Model *model = obj->models[obj->curModel];
    void **anims; //this is a struct of some sort.
    if(model->unk66) {
        anims = model->animations;
        if (!anims) { }
        func_800255F8(obj, model, anims, a1, (short) a2); //probably ObjSetBlendMove
        //not certain about the order/count of params here.
    }
}
#endif

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_8002559C.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_800255F8.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80025780.s")

#pragma GLOBAL_ASM("asm/nonmatchings/object/func_80025CD4.s")