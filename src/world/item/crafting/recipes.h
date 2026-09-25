
#ifndef MCPSP_WORLD_ITEM_CRAFTING_RECIPES_H
#define MCPSP_WORLD_ITEM_CRAFTING_RECIPES_H

#include "world/item/crafting/recipe.h"
#include <initializer_list>

enum {
    DYE_BLACK = 0, DYE_RED = 1, DYE_GREEN = 2, DYE_BROWN = 3, DYE_BLUE = 4,
    DYE_PURPLE = 5, DYE_CYAN = 6, DYE_SILVER = 7, DYE_GRAY = 8, DYE_PINK = 9,
    DYE_LIME = 10, DYE_YELLOW = 11, DYE_LIGHT_BLUE = 12, DYE_MAGENTA = 13,
    DYE_ORANGE = 14
};
static inline short clothData(int dyeAux) { return (short)(~dyeAux & 0xf); }

class Recipes {
public:
    static Recipes* getInstance();
    const std::vector<Recipe*>& getRecipes() const { return recipes; }

    struct Type { char c; ItemInstance ing; };

    void addShapedRecipe(const ItemInstance& result, const char* r0,
                         std::initializer_list<Type> types);
    void addShapedRecipe(const ItemInstance& result, const char* r0, const char* r1,
                         std::initializer_list<Type> types);
    void addShapedRecipe(const ItemInstance& result, const char* r0, const char* r1, const char* r2,
                         std::initializer_list<Type> types);
    void addShapelessRecipe(const ItemInstance& result,
                            std::initializer_list<ItemInstance> ingredients);

private:
    Recipes();
    ~Recipes();
    void addShapedRows(const ItemInstance& result, const char* const* rows, int height,
                       std::initializer_list<Type> types);

    static Recipes* instance;
    std::vector<Recipe*> recipes;
};

inline Recipes::Type TILE(char c, short tileId) { return { c, ItemInstance(tileId, 1, Recipe::ANY_AUX_VALUE) }; }
inline Recipes::Type ITEM(char c, short itemId) { return { c, ItemInstance(itemId, 1, 0) }; }
inline Recipes::Type INST(char c, short id, short aux) { return { c, ItemInstance(id, 1, aux) }; }

#endif
