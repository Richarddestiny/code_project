height= 720;
width= 1280;
a = ls('./*.yuv');
[nb_frame,~] = size(a);
mkdir('./RGB/');

for i_frame = 1:nb_frame
    yuv_path = ['./', a(i_frame,:)];
    fid = fopen(yuv_path, 'r');
    YUV = fread(fid, height * width * 1.5, 'uint8');
    fclose(fid);
    img = YUV2RGB(YUV, height, width);
    imwrite(img,['./RGB/',a(i_frame,1:end-4),'.jpg']);
end