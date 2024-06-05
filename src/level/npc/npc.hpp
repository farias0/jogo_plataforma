#include <raylib.h>

#include "../level.hpp"
#include "../../animation.hpp"


#define NPC_PERSISTENCE_ID          "npc"


class Npc : public Level::Entity {

public:

    static Npc *Add();

    static Npc *Add(Vector2 pos);

    static void CheckAndAdd(Vector2 pos, int interactionTags);

    virtual void Tick() override;

    virtual void Reset() override;

    void PersistenceParse(const std::string &data) override;
    std::string PersistanceSerialize() override;

    std::string PersistanceEntityID() {
        return NPC_PERSISTENCE_ID;
    }


private:

    bool isFalling;
};
