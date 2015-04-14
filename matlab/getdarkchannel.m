function dark=getdarkchannel(I, win_size)
if (~exist('win_size','var'))
    win_size = 1;
end
if (isempty(win_size))
    win_size = 1;
end

[h,w,c] = size(I);
dark = zeros(h, w);
mI = min(I, [], 3);
for y = 1:h
    for x = 1:w
        winI = mI(max(1, y-win_size):min(h, y+win_size),  max(1, x-win_size):min(w, x+win_size));
        dark(y, x) = min(min(winI));
    end;
end;
%figure, imshow(dark/255);
