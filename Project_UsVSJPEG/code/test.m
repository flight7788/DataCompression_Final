clear all
close all

JPEG_quality_min = 0;
JPEG_quality_step = 5;
JPEG_quality_max = 100;

resize_min_rate = 0.1;
resize_step_rate = 0.1;
resize_max_rate = 0.9;

half_quality_min = 10;
half_quality_step= 5;
half_quality_max = 100;

error_quality_min = 10;
error_quality_step = 1;
error_quality_max = 100;

dataset_path = "./testimg/";
output_path = "./results/";
file_name = "AH_deg0_pit0";
in_ex_name = ".bmp";
out_ex_name = ".jpg";
output_path = output_path + file_name + "/";
mkdir(output_path);

input = double(imread(dataset_path + file_name + in_ex_name));
[rows, cols, channels] = size(input);

%% directly JPEG
file = fopen(output_path+'JPEG_'+file_name+'_results.csv','a');

mkdir(output_path + "JPEG/");
fprintf(file, 'JPEG\n');    
fprintf(file, 'Quality, File size (KB), MSE, PSNR\n');    

for JPEG_quality = JPEG_quality_min : JPEG_quality_step : JPEG_quality_max
    %% Encode
    imwrite(uint8(input), output_path + "JPEG/" + "JPEG_" + string(JPEG_quality) + "_" + file_name + ".jpg",'Quality',JPEG_quality);
    %% Decode
    JPEG = double(imread(output_path + "JPEG/" + "JPEG_" + string(JPEG_quality) + "_" + file_name + ".jpg"));
    %% Compute
    MSE_JPEG = sum(sum(sum((input-JPEG).^2))) / (512*512*3);
    PSNR_JPEG = 10 * log10( 255^2 / MSE_JPEG);
    size_JPEG =  dir(output_path + "JPEG/" + "JPEG_" + string(JPEG_quality) + "_" + file_name + ".jpg").bytes / 1024;
    %% write file
    fwrite(file, string(JPEG_quality) + ", " + size_JPEG + ", " + string(MSE_JPEG) + ", " + string(PSNR_JPEG));
    fprintf(file,'\n');     
end
fclose(file);


%% my method
file = fopen(output_path+'MY_'+file_name+'_results.csv','a');
fprintf(file, 'MY_Coding\n');    
fprintf(file, 'Resize rate, Half quality, Error quality, Half size (KB), Error size (KB), Total size(KB), MSE, PSNR\n');   

for resize_rate = resize_min_rate : resize_step_rate : resize_max_rate
    sub_rate_dir = "resize_rate_" + string(resize_rate) + "/";
    mkdir(output_path + sub_rate_dir);

    for half_quality = half_quality_min : half_quality_step : half_quality_max
        sub_half_dir = "half_quality_" + string(half_quality) + "/";
        mkdir(output_path + sub_rate_dir + sub_half_dir);
        cur_dir = output_path + sub_rate_dir + sub_half_dir;
        for error_quality = error_quality_min : error_quality_step: error_quality_max
            half_prefix_file_name = "J_half_" + string(resize_rate) + "_" + string(half_quality) + "_" +error_quality + "_";
            error_prefix_file_name = "J_error_" + string(resize_rate) + "_" + string(half_quality) + "_" +error_quality + "_";
            %% encode
            half = imresize(input, resize_rate);
           
            imwrite(uint8(round(half)), cur_dir + half_prefix_file_name + file_name + ".jpg",'Quality', half_quality);

            J_half = double(imread(cur_dir + half_prefix_file_name + file_name + ".jpg"));
            J_full = imresize(J_half, [rows,cols]);

            error = input - J_full; % error: double
                       
            min_e = min(min(min(error)))
            max_e = max(max(max(error)))
            
            if min_e >= 0
                error_mapping = error;
                mode = 1
            elseif max_e < 255 - abs(min_e)
                error_mapping = error + abs(min_e);
                mode = 2
            else
                mode = 3
                error_mapping = (error + 255) / 2;
            end
            h_error_mapping = imresize(error_mapping, 0.8);
            imwrite(uint8(round(h_error_mapping)), cur_dir + error_prefix_file_name + file_name + ".jpg", 'Quality', error_quality);

            %error_mapping = round(error_mapping);
            
            %imwrite(uint8(error_mapping), cur_dir + error_prefix_file_name + file_name + ".jpg", 'Quality', error_quality);
           
            
            %% decode
            J_error = double(imread(cur_dir + error_prefix_file_name + file_name + ".jpg"));
            J_error = imresize(J_error, [rows,cols]);
            if mode == 1
            elseif mode == 2
                J_error = J_error - abs(min_e);
            elseif mode == 3
                J_error = J_error * 2 - 255;
            else
                info = "error"
            end
            J_half = double(imread(cur_dir + half_prefix_file_name + file_name + ".jpg"));
            J_refull = imresize(J_half, [rows, cols]);
            decode_ret = J_refull + J_error;
            decode_ret = round(decode_ret);
          
            %% Compute c
            MSE_MY = sum(sum(sum((input-decode_ret).^2))) / (512*512*3);
            PSNR_MY = 10 * log10( 255^2 / MSE_MY);
            size_Half =  dir(cur_dir + half_prefix_file_name + file_name + ".jpg").bytes / 1024;
            size_Error = dir(cur_dir + error_prefix_file_name + file_name + ".jpg").bytes / 1024;
            size_total = size_Half + size_Error;
            %%
           fwrite(file, string(resize_rate) + ", " + string(half_quality) + ", " + string(error_quality)+ ", " + string(size_Half) + ", " +string(size_Error) + "," + string(size_total)+","+string(MSE_MY)+"," + string(PSNR_MY));
           fprintf(file,'\n');    
        end
    end
end
fclose(file);