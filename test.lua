x, y = buffer:get_size();
for i = 1, x, 1 do
    for j = 1, y, 1 do
        buffer:set(i, j, i % 10, { type = "bit", r = 1, g = 0, b = 0 })
    end
end
