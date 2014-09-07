function [res dark t] = dehazing(I, win_size, t0);
if (~exist('win_size','var'))
    win_size = 1;
end
if (~exist('t0', 'var'))
    t0 = 0.25;
end;

dark = getdarkchannel(I, win_size);

A = getairlight(dark);

I_filter = FilterImage(I);

t = gettransmission(A, 0.9375, I_filter);

t = medfilt2(t);

res = recover(I, A, t, t0)/255;


