function J = recover(I, A, t, t0)
if(~exist('t0', 'var'))
    t0 = 0.25;
end;
if(isempty(t0))
    t0 = 0.25;
end;

[h, w, c] = size(I);
J = zeros(h, w, c);
for i = 1:h
    for j = 1:w
        final_t = max(t(i,j), t0);
        J(i, j, :) = (I(i, j, :)-A)/final_t+A;
        for k = 1:c
            if J(i,j,k)<=0
                J(i,j,k) = 0;
            elseif J(i,j,k)>255
                J(i,j,k) = 255;
            end;
        end;
    end;
end;
%test;