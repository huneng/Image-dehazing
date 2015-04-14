function t = gettransmission(A, omega, I)
if(~exist('omega', 'var'))
    omega = 0.9375;
end;
if(isempty(omega))
    omega = 0.9375;
end;
[h, w, c] = size(I);
t = zeros(h, w);

IA = I/A;
t(:, :) = 1-omega*min(IA, [], 3);
