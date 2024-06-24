items = {
    {name = "Item_Pickaxe.png", price = 1},
    {name = "Item_Sword.png", price = 1},
    {name = "Item_Hammer.png", price = 1},
    {name = "Item_Arrow.png", price = 1},
    {name = "Item_28.png", price = 1}
}

function GetItemPrice(item_name)
    for _, item in ipairs(items) do
        if item.name == item_name then
            return item.price
        end
    end
    return nil
end