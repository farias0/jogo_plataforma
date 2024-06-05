#include "npc.hpp"
#include "princess.hpp"
#include "../../render.hpp"
#include "../../editor.hpp"


#define NPC_TYPE_DEFAULT        PRINCESS_PERSISTENCE_ID

#define NPC_FALL_RATE           7.0f


std::map<std::string, void (*)(Vector2, int)> INpc::npcTypeMap;


void INpc::Initialize() {
    
    /*
        TODO 
            - generalize the PERSISTENCE_IDs to an universal Entity Type ID
            - have the PeristenceEntityID() equivalent method be static and virtual to maybe all level entities
    */

    npcTypeMap = {
        { PRINCESS_PERSISTENCE_ID, &Princess::CheckAndAdd }
    };
}

void INpc::CheckAndAdd(Vector2 pos, int interactionTags) {

    if (!(interactionTags & EDITOR_INTERACTION_CLICK)) return;


    std::string npcType = NPC_TYPE_DEFAULT; 
    

    if (auto collidedEntity = Level::CheckCollisionWithAnyEntity(pos)) {

        if (collidedEntity->tags & Level::IS_NPC && interactionTags & EDITOR_INTERACTION_ALT) {
            
            npcType = ((INpc *)collidedEntity)->PersistanceEntityID();
            
            Level::EntityDestroy(collidedEntity);

            // next npc type
            for (auto it = npcTypeMap.begin(); it != npcTypeMap.end(); it++) {
                if (npcType != it->first) continue;

                if (std::next(it) == npcTypeMap.end()) { // last item
                    it = npcTypeMap.begin();
                } else {
                    it++;
                }

                npcType = it->first;
            }

            Render::PrintSysMessage("Mudando NPC para " + npcType);
        }
        else {

            // Clicked over a non-NPC entity
            return;
        }

    }
    
    (npcTypeMap.at(npcType))(pos, interactionTags);
}

void INpc::Tick()
{

    if (Level::STATE->concludedAgo >= 0) return;

    if (isFalling) {
        if (hitbox.y + hitbox.height > FLOOR_DEATH_HEIGHT) {
            isFalling = false;
            Render::PrintSysMessage("NPC não encontrou geometria");
            TraceLog(LOG_WARNING, "Falling NPC didn't find the ground");
        }
        else if (auto groundBeneath = Level::GetGroundBeneath(this)) {
            hitbox.y = groundBeneath->hitbox.y - hitbox.height;
            isFalling = false;
        }
        else {
            hitbox.y += NPC_FALL_RATE;
        }
    }
}

void INpc::Reset() {

    Level::Entity::Reset();
    isFalling = true;
}

void INpc::PersistenceParse(const std::string &data) {

    Level::Entity::PersistenceParse(data);
    // int id = std::stoi(persistenceReadValue(data, "textId")); // TODO NPC ID
}

std::string INpc::PersistanceSerialize() {
    
    std::string data = Level::Entity::PersistanceSerialize();
    // TODO add NPC ID
    return data;
}
