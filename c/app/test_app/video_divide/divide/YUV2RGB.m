%% YUV2RGB  format:411
function [RGB] = YUV2RGB(yuv, height, width)

y = reshape(yuv(1 : width * height), width, height)';
u = reshape(yuv(width * height + 1 : width * height * 1.25), width / 2, height / 2)';
v = reshape(yuv(width * height * 1.25 + 1 : width * height * 1.5), width / 2, height / 2)';

y=double(y);
u=double(u - 128);
v=double(v - 128);

RGB = zeros(height, width, 3);

RGB(1:2:end,1:2:end,1) = y(1:2:end,1:2:end) + 1.140 * v;
RGB(2:2:end,1:2:end,1) = y(2:2:end,1:2:end) + 1.140 * v;
RGB(2:2:end,2:2:end,1) = y(2:2:end,2:2:end) + 1.140 * v;
RGB(1:2:end,2:2:end,1) = y(1:2:end,2:2:end) + 1.140 * v;

RGB(1:2:end,1:2:end,2) = y(1:2:end,1:2:end) - 0.394 * u - 0.581 * v;
RGB(2:2:end,1:2:end,2) = y(2:2:end,1:2:end) - 0.394 * u - 0.581 * v;
RGB(2:2:end,2:2:end,2) = y(2:2:end,2:2:end) - 0.394 * u - 0.581 * v;
RGB(1:2:end,2:2:end,2) = y(1:2:end,2:2:end) - 0.394 * u - 0.581 * v;

RGB(1:2:end,1:2:end,3) = y(1:2:end,1:2:end) + 2.032 * u;
RGB(2:2:end,1:2:end,3) = y(2:2:end,1:2:end) + 2.032 * u;
RGB(2:2:end,2:2:end,3) = y(2:2:end,2:2:end) + 2.032 * u;
RGB(1:2:end,2:2:end,3) = y(1:2:end,2:2:end) + 2.032 * u;
RGB = uint8(RGB); 

end
