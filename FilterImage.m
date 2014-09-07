function I_filter = FilterImage(I)
D = 80;
[h, w, c] = size(I);

I_filter = I;

for y = 1:h
    y1 = min(y+1, h);
    y0 = max(1, y-1);
    for x = 1:w
        x1 = min(x+1, w);
        x0 = max(1, x-1);
        for z = 1:c
            if abs(I(y1, x1, z)-I(y0, x0, z))>=D || abs(I(y0, x1, z)-I(y1, x0, z))>=D
                I_filter(y, x, z) = 4*I(y, x, z)+ ...
                    2*(I(y1, x1, z)+I(y0, x0, z)+I(y1, x0, z)+I(y0, x1, z))+ ...
                    (I(y, x1, z)+I(y, x0, z)+I(y1, x, z)+I(y0, x, z));
                I_filter(y, x, z) = I_filter(y, x, z) /16;
            elseif abs(I(y, x1, z)-I(y, x0, z))>=D || abs(I(y0, x, z)-I(y1, x, z))>=D
                I_filter(y, x, z) = 4*I(y, x, z)+ ...
                    (I(y1, x1, z)+I(y0, x0, z)+I(y1, x0, z)+I(y0, x1, z))+ ...
                    2*(I(y, x1, z)+I(y, x0, z)+I(y1, x, z)+I(y0, x, z));
                I_filter(y, x, z) = I_filter(y, x, z) /16;
            else
                I_filter(y, x, z) = I(y, x, z)+ ...
                    (I(y1, x1, z)+I(y0, x0, z)+I(y1, x0, z)+I(y0, x1, z))+ ...
                    (I(y, x1, z)+I(y, x0, z)+I(y1, x, z)+I(y0, x, z));
                I_filter(y, x, z) = I_filter(y, x, z) /9;
            end;
        end;
    end;
end;