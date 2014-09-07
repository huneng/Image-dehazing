function A = getairlight(dark)
[h, w] = size(dark);
A = zeros(h, w, 3);
darkT = reshape(dark, 1, h*w);
darkT = sort(darkT, 'descend');

A = darkT(1);
            
            
