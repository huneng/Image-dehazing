clear;
close all;
src_dir = 'origin\';
dark_dir= 'dark\';
tras_dir= 'transimission\';
dst_dir = 'results\';

file_list = dir(src_dir);
len = length(file_list);

win_size = 7;
t0=0.1;
mkdir(dst_dir);
mkdir(dark_dir);
mkdir(tras_dir);

for i = 3:len
    img_name = file_list(i).name;
    
    img_path = strcat(src_dir, img_name)
    
    I = double(imread(img_path));
    
    [res, dark, t] = dehazing(I, win_size, t0);
    
    
    [out_name, fmt]=getOutputFilePath2(img_name, '', dst_dir);
    imwrite(res, out_name, fmt);
    [out_name, fmt]=getOutputFilePath2(img_name, '', dark_dir);
    imwrite(dark/255, out_name, fmt);
    [out_name, fmt]=getOutputFilePath2(img_name, '', tras_dir);
    imwrite(t, out_name, fmt);
end;
