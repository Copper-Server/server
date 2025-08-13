[ ] Refactor storage::world_data and base_object::entity to be more abstract,
    the world should operate only with entities,
    the entity is like proxy to pass data about surrounding area and changes to players or AI, each entity holds information about loaded chunks and last tick of handled changes to sync with world
    the if entity holds client, it flushes all changes to client and client sends movement changes to entity

    [x] Make interface for entity
    [x] Change world_data to handle entities
    [ ] Use new interface in player and add handlers 
        [ ] Implement chunk encoding and send to player
            [ ] fix encoding
