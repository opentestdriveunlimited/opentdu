#include "shared.h"
#include "game_object.h"

GameObject::GameObject()
    : objectID( 0 )
    , parentID( 0 )
    , netID( 0 )
    , timeSinceLastUpdate( 0.0f )
    , worldLocation()
    , pNetOwner( nullptr )
{
}

GameObject::~GameObject()
{
}
