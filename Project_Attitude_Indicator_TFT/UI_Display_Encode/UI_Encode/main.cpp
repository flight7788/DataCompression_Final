#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <iomanip>
#include <direct.h>
#include <fstream>
#include <vector>
#include <string>

using namespace cv;
using namespace std;

uint16_t Color888to565(Vec3b rgb) {
	uint16_t d = ((uint16_t)(rgb[0] >> 3) << 11) + //R G B
				 ((uint16_t)(rgb[1] >> 2) << 5) +
				  (uint16_t)(rgb[2] >> 3);
	return d;
}

Vec3b Color565to888(uint16_t rgb) {
	Vec3b d = {
		(unsigned char)((rgb & 0xF800) >> 8),
		(unsigned char)((rgb & 0x07E0) >> 3),
		(unsigned char)((rgb & 0x001f) << 3)
	};
	return d;
}

vector<uint16_t> Find_ColorTable(Mat Input) {
	vector <uint16_t> color;
	for (int row = 0; row < Input.rows; row++) {
		for (int col = 0; col < Input.cols; col++) {
			uint16_t d = Color888to565(Input.at<Vec3b>(row, col));
			bool flag = true;
			for (int i = 0; i < color.size(); i++) {
				if (d == color[i]) {
					flag = false;
					break;
				}
			}
			if(flag)
				color.push_back(d);
		}
	}
	return color;
}

vector<uint8_t> Compression_Encode(Mat Input, vector<uint16_t> table) {
	vector <uint8_t> buffer;
	uint32_t index = 0, len = Input.rows * Input.cols;
	while(index < len) {
		uint16_t color_rgb = Color888to565(Input.ptr<Vec3b>(index / Input.rows)[index % Input.cols]);
		uint32_t index_new = index + 1, count = 1; 
		while ((index_new < len) && (color_rgb == Color888to565(Input.ptr<Vec3b>(index_new / Input.rows)[index_new % Input.cols]))) {
			count++;
			index_new++;
		}
		//cout << "count:" << count << endl;
		index = index_new;
		buffer.push_back(count >> 8);
		buffer.push_back(count & 0x00FF);
		bool flag = false;
		for (int index = 0; index < table.size(); index++) {
			if (table[index] == color_rgb) {
				buffer.push_back(index);
				flag = true;
				break;
			}
		}
		if (flag == false) cout << "Not Existing !!" << endl;
	}
	return buffer;
}

Mat Compression_Decode(vector<uint8_t> Buf, Size In, vector<uint16_t> table) {
	Mat Out = Mat::zeros(In.height, In.width, CV_8UC3);
	uint32_t index = 0;
	for (uint32_t index_Buf = 0; index_Buf < Buf.size(); index_Buf += 3) {
		uint16_t N_times = (Buf[index_Buf + 0] << 8) | Buf[index_Buf + 1];
		uint16_t Color   = table[Buf[index_Buf + 2]];
		uint32_t Temp_index = index;
		while (index < (Temp_index + N_times)) {
			Out.at<Vec3b>(index / In.height, index % In.width) = Color565to888(Color);
			index++;
		}
	}
	return Out;
}

bool ColorSelect(Vec3b ChoseColor, Vec3b SetColor, uint8_t range) {
	for (int channel = 0; channel < 3; channel++) {
		int upper = (((int)SetColor[channel] + range) > 255) ? 255 : (SetColor[channel] + range),
			lower = (((int)SetColor[channel] - range) <  0)  ?   0 : (SetColor[channel] - range);
		//cout << "channel:" << dec << channel << ", upper:" << dec << upper << ", lower:" << dec << lower << endl;
		if (!((ChoseColor[channel] >= lower) && (ChoseColor[channel] <= upper))) {
			return false;
		}
	}
	return true;
}

void ChangeColor(Mat &data) {
	int range = 70;
	for (int row = 0; row < data.rows; row++) {
		for (int col = 0; col < data.cols; col++) {
			Vec3b color = data.ptr<Vec3b>(row)[col];
			if (ColorSelect(color, { 240, 160, 80 }, range)) {
				data.ptr<Vec3b>(row)[col] = { 240, 160, 80 };
			}
			else if (ColorSelect(color, { 248, 244, 232 }, range)) {
				data.ptr<Vec3b>(row)[col] = { 248, 244, 232 };
			}
			else if (ColorSelect(color, { 56, 100, 128 }, range)) {
				data.ptr<Vec3b>(row)[col] = { 56, 100, 128 };
			}
			else if (ColorSelect(color, { 48, 240, 248 }, range)) {
				data.ptr<Vec3b>(row)[col] = { 48, 240, 248 };
			}
			else if (ColorSelect(color, { 80, 108, 96 }, range)) {
				data.ptr<Vec3b>(row)[col] = { 80, 108, 96 };
			}
			else {
				data.ptr<Vec3b>(row)[col] = { 248, 244, 232 };
			}
		}
	}
}

void ShowColorTable(vector <uint16_t> Table) {
	cout << "Color Table :" << endl;
	for (int i = 0; i < Table.size(); i++) {
		Vec3b color = Color565to888(Table[i]);
		cout << " " << dec << i + 1 << ".Color R:" << setfill(' ') << setw(3) << dec << (int)color[0] 
			                              << " G:" << setfill(' ') << setw(3) << dec << (int)color[1] 
										  << " B:" << setfill(' ') << setw(3) << dec << (int)color[2] << "  ";
		cout << "Hex RGB:" << setfill('0') << setw(4) << hex << Table[i] 
			    << " BGR:" << setfill('0') << setw(4) << hex << (((Table[i] & 0x001f) << 11) | (Table[i] & 0x07e0) | ((Table[i] & 0xF800) >> 11))
			    << endl;
	}
}

void showEncode(vector<uint8_t> buf) {
	cout << "buf: " << endl;
	for (int i = 0; i < buf.size(); i++) {
		cout << setfill('0') << setw(2) << right << hex << (int)buf[i] << " ";
	}
	cout << endl << "size:" << dec << buf.size() << endl;
}

void ShowCurrentSize(vector<uint8_t> Aft_Buf, Mat Bef_Buf) {
	cout << "Size (Origin): " << ((uint64_t)Bef_Buf.rows * Bef_Buf.cols * 2) / 1024.0 << " KB" << endl;
	cout << "Size (Compression): " << Aft_Buf.size() / 1024.0 << " KB" << endl;
	cout << "Compression Rate: " << (double)((uint64_t)Bef_Buf.rows * Bef_Buf.cols * 2) / Aft_Buf.size() << endl;
}

Mat ReadBMPFile(int roll, int pitch) {
	char file_name[30];
	if (pitch < 0) snprintf(file_name, sizeof(file_name), "AH_deg%d_pitN%d", roll, abs(pitch));
	else snprintf(file_name, sizeof(file_name), "AH_deg%d_pit%d", roll, abs(pitch));
	string name = file_name;
	string file_dir = "D:\\Users/Daniel/Desktop/UI_Display_Encode/Input/" + name + ".bmp";
	Mat data = imread(file_dir, IMREAD_COLOR);
	return data;
}

void WriteBinFile(vector<uint8_t> buf, int roll, int pitch) {
	FILE* fp;
	char file[30];
	snprintf(file, sizeof(file), "P%c%d/R%d", (pitch < 0) ? 'N' : '_', abs(pitch), roll);
	string f_name = file;
	string dst_dir = "D:\\Users/Daniel/Desktop/UI_Display_Encode/Output/" + f_name + ".bin";
	fopen_s(&fp, dst_dir.c_str(), "wb");
	uint8_t* buf_array = &buf[0];
	fwrite(buf_array, 1, buf.size(), fp);
	fclose(fp);
}

void WriteJPGFile(Mat data, int roll, int pitch) {
	char file[30];
	snprintf(file, sizeof(file), "P%c%d/R%d", (pitch < 0) ? 'N' : '_', abs(pitch), roll);
	string f_name = file;
	string dir = "D:\\Users/Daniel/Desktop/UI_Display_Encode/Output_JPG/" + f_name + ".jpg";
	imwrite(dir, data);
}

uint32_t ReadFileSize(String File) {
	FILE* fp;
	fopen_s(&fp, File.c_str(), "rb");
	fseek(fp, 0, SEEK_END);
	const uint32_t size = ftell(fp);
	fclose(fp);
	return size;
}

void WriteSizetoCSV(void) {
	ofstream myfile;
	myfile.open("D:\\Users/Daniel/Desktop/UI_Display_Encode/Result.csv");
	myfile << "Pitch,Roll,OriginSize(KB),OurSize(KB),JPEGSize(KB),CR(Our),CR(JPEG),\n";
	int pitch = 0, roll = 0;

	for (pitch = -90; pitch <= 90; pitch++) {
		for (roll = 0; roll < 360; roll++) {	
			char file[30];
			snprintf(file, sizeof(file), "P%c%d/R%d", (pitch < 0) ? 'N' : '_', abs(pitch), roll);
			string FileName = file;
			Mat Input = ReadBMPFile(roll, pitch);

			uint32_t OriginSize = Input.cols * Input.rows * 2,
				OurSize = ReadFileSize("D:\\Users/Daniel/Desktop/UI_Display_Encode/Output/" + FileName + ".bin"),
				JPEGSize = ReadFileSize("D:\\Users/Daniel/Desktop/UI_Display_Encode/Output_JPG/" + FileName + ".jpg");

			double OriginSize_KB = OriginSize / 1024.0,
				      OurSize_KB =    OurSize / 1024.0,
				     JPEGSize_KB =   JPEGSize / 1024.0,
				          Our_CR = (double)OriginSize / OurSize,
				         JPEG_CG = (double)  JPEGSize / OurSize;
			char data[100];
			snprintf(data, sizeof(data), "%d,%d,%f,%f,%f,%f,%f,\n", pitch, roll, OriginSize_KB, OurSize_KB, JPEGSize_KB, Our_CR, JPEG_CG);
			myfile << data;

			cout << " Pitch:" << setfill(' ') << setw(3) << right << dec << pitch 
				 << ", Roll:" << setfill(' ') << setw(3) << right << dec << roll 
				 << "   OK!!" << endl;
			/*
			cout << " Pitch:" << pitch << ", Roll:" << roll << endl;
			cout << " OriginSize:" << OriginSize_KB << " KB" << endl;
			cout << "    OurSize:" << OurSize_KB << " KB" << endl;
			cout << "   JPEGSize:" << JPEGSize_KB << " KB" << endl;
			cout << "    CR(Our):" << Our_CR << endl;
			cout << "   CR(JPEG):" << JPEG_CG << endl;
			cout << "=====================================================" << endl << endl;
			//*/
		}
	}
	myfile.close();
}


int main(void) {
	int pitch = 0, roll = 0;
	uint32_t count = 0;
	Mat data = ReadBMPFile(roll, pitch);
	ChangeColor(data);
	vector<uint16_t> table = Find_ColorTable(data);

	//WriteSizetoCSV();
	//ShowColorTable(table);
	
	
	
	/*
			FILE* fp;
			char file[100];
			snprintf(file, sizeof(file), "D:\\Users/Daniel/Desktop/UI_Display_Encode/Output/P%c%d/", (pitch < 0) ? 'N' : '_', abs(pitch));
			_mkdir(file);
			//*/
	/*
	for (pitch = -90; pitch <= 90; pitch++) {
		for (roll = 0; roll < 360; roll++) {
			data = ReadBMPFile(roll, pitch);
			ChangeColor(data);
			//vector<uint8_t> buf = Compression_Encode(data, table);
			//WriteBinFile(buf, roll, pitch);
			WriteJPGFile(data, roll, pitch);
			cout << "File" << dec << count << " is OK !!" << endl;
			count++;			
		}
	}
	//*/
	

	


	
	//waitKey(0);
	return 0;
}