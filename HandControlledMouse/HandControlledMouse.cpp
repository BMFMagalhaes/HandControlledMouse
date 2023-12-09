// HandControlledMouse.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define __STDC_WANT_LIB_EXT1__ 1
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <WinSock2.h>
#include <iostream>
#include "LeapHeader.h"
#include "tensorflow/c/c_api.h"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <onnxruntime_cxx_api.h>
#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 56234
void save_hand_data_csv(const LEAP_HAND* hand, FILE* file);
void save_hand_data_csv2(const LEAP_HAND* hand, FILE* file);
void save_hand_position(const LEAP_HAND* hand, FILE* file);

int64_t lastFrameID = 0; //The last frame received
int64_t lastImageID = 0; //The last image received

//using namespace std;

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

// Function to read CSV file and extract min and max values
/*void readMinMaxValues(const std::string& filename, const std::string& filename2, std::vector<float>& minValues, std::vector<float>& maxValues) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		std::cout << "Failed to open file: " << filename << std::endl;
		return;
	}

	std::string line;
	while (std::getline(file, line)) {
		std::istringstream iss(line);
		float minValue, maxValue;
		if (iss >> minValue >> maxValue) {
			minValues.push_back(maxValue);
		}
	}
	file.close();

	std::ifstream file(filename2);
	if (!file.is_open()) {
		std::cout << "Failed to open file: " << filename2 << std::endl;
		return;
	}

	std::string line;
	while (std::getline(file, line)) {
		std::istringstream iss(line);
		float minValue, maxValue;
		if (iss >> minValue >> maxValue) {
			maxValues.push_back(maxValue);
		}
	}
	file.close();
}

void normalizeData(const LEAP_HAND* data, const std::vector<float>& minValues, const std::vector<float>& maxValues, std::vector<float>& finalValues) {
	finalValues.push_back(data->grab_angle);
	finalValues.push_back(data->grab_strength);
	finalValues.push_back(data->pinch_distance);
	finalValues.push_back(data->pinch_strength);
	finalValues.push_back(data->type);
	for (LEAP_HAND& hand : data) {
		// Normalize attribute1
		hand.attribute1 = (hand.attribute1 - minValues[0]) / (maxValues[0] - minValues[0]);

		// Normalize attribute2
		hand.attribute2 = (hand.attribute2 - minValues[1]) / (maxValues[1] - minValues[1]);

		// ... normalize other attributes
	}
}*/


int main()
{
	/*const ORTCHAR_T* modelPath = ORT_TSTR("");
	Ort::Env env_;
	Ort::RunOptions runOptions_;
	Ort::SessionOptions SessionOptions_;

	Ort::Session session_ = Ort::Session(env_, modelPath, SessionOptions_);

	// define I/O shape
	const array<int64_t,2> inputShape = { 1, 259};
	const array<int64_t, 2> outputShape = { 1, 6 };

	// define I/O array
	array<float, 259> input;
	array<float, 6> results;
	vector<float> e;

	LEAP_TRACKING_EVENT* frame = GetFrame();
	if (frame && (frame->tracking_frame_id > lastFrameID)) {
		lastFrameID = frame->tracking_frame_id;
		for (uint32_t h = 0; h < frame->nHands; h++) {
			LEAP_HAND* hand = &frame->pHands[h];
			input = { hand->visible_time, hand->grab_angle, hand->grab_strength, hand->pinch_distance, hand->pinch_strength, hand->type == eLeapHandType_Left ? 1 : 0 }
		}
	}

	// define I/O Tensor
	// It hold the array pointer internally
	// Don't delete the array while the tensor is alive
	// If use vector, don't reallocate memory after creating the tensor
	auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
	auto inputTensor = Ort::Value::CreateTensor<float>(memory_info, input.data(), input.size(), inputShape.data(), inputShape.size());
	auto outputTensor = Ort::Value::CreateTensor<float>(memory_info, results.data(), results.size(), outputShape.data(), outputShape.size());


	// define I/O names
	Ort::AllocatorWithDefaultOptions ort_alloc;
	Ort::AllocatedStringPtr inputName = session_.GetInputNameAllocated(0, ort_alloc);
	Ort::AllocatedStringPtr outputName = session_.GetOutputNameAllocated(0, ort_alloc);
	array<Ort::AllocatedStringPtr&, 1> inputNames = { inputName };
	array<Ort::AllocatedStringPtr&, 1> outputNames = { outputName };

	// run inference
	try
	{
		session_.Run(Ort::RunOptions{ nullptr }, inputNames.data(), &inputTensor, 1, outputNames.data(), &outputTensor, 1);
	}
	catch (Ort::Exception& e)
	{
		cout << e.what() << endl;
		//return 1;
	}*/
	// este bool serve para decidir se a ligação é suposto funcionar só para as features ou para as imagens
	bool is_Image = false;
	//loadModel();
	// Leap Initialization
	LEAP_CONNECTION* connection = OpenConnection();
	if (is_Image) {
		LeapSetPolicyFlags(*connection, eLeapPolicyFlag_Images, 0);
	}
	while (!IsConnected)
		millisleep(100); //wait a bit to let the connection complete
	printf("Connected.");
	LEAP_DEVICE_INFO* deviceProps = GetDeviceProperties();
	if (deviceProps)
		printf("Using device %s.\n", deviceProps->serial);

	// Opening of TCP Server Socket to comunicate with Python Client
	WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize winsock." << std::endl;
        return 1;
    }

    SOCKET server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    char message[1024];

    // Create a TCP socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        return 1;
    }

    // Set up the server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(SERVER_PORT);

    // Bind the socket to the server address
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind socket." << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_socket, 1) == SOCKET_ERROR) {
        std::cerr << "Failed to listen for connections." << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server listening on port " << SERVER_PORT << std::endl;

    // Accept a client connection
    int client_address_len = sizeof(client_address);
    client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_len);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Failed to accept client connection." << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Client connected." << std::endl;

	for (;;) {
		// Receive data from the client
		int bytes_received = recv(client_socket, message, sizeof(message), 0);
		if (bytes_received == SOCKET_ERROR) {
			std::cerr << "Failed to receive data from client." << std::endl;
			closesocket(client_socket);
			closesocket(server_socket);
			WSACleanup();
			return 1;
		}

		// Print the received message
		std::cout << "Received message from client: " << message << std::endl;

		if (!is_Image) {
			//Para os frames
			LEAP_TRACKING_EVENT* frame = GetFrame();
			if (frame && (frame->tracking_frame_id > lastFrameID)) {
				lastFrameID = frame->tracking_frame_id;
				printf("Frame %lli with %i hands.\n", (long long int)frame->tracking_frame_id, frame->nHands);
				FILE* file;
				errno_t error_code;
				error_code = fopen_s(&file, "C:\\teste\\new-row.csv", "w");
				if (error_code != 0) {
					printf("Error! Failed to open file in r mode!");
				}
				else {
					printf("I opened myfile.c in r mode!\n");
				}
				if (frame->nHands == 1) {
					LEAP_HAND* hand = &frame->pHands[0];
					save_hand_data_csv(hand, file);
				}
				/*for (uint32_t h = 0; h < frame->nHands; h++) {
					LEAP_HAND* hand = &frame->pHands[h];
					save_hand_data_csv(hand, file);
				}*/
				fclose(file);
				if (frame->nHands == 0 || frame->nHands == 2) {
					if (std::remove("C:\\teste\\new-row.csv") == 0) { printf("deu"); };
				}
				
			}
			const char* reply = "charPtr";
			if (send(client_socket, reply, strlen(reply), 0) == SOCKET_ERROR) {
				std::cerr << "Failed to send data to client." << std::endl;
				closesocket(client_socket);
				closesocket(server_socket);
				WSACleanup();
				return 1;
			}
		}

		if (is_Image) {
			LEAP_TRACKING_EVENT* frame = GetFrame();
			if (frame && (frame->tracking_frame_id > lastFrameID)) {
				lastFrameID = frame->tracking_frame_id;
				printf("Frame %lli with %i hands.\n", (long long int)frame->tracking_frame_id, frame->nHands);
				FILE* file;
				errno_t error_code;
				error_code = fopen_s(&file, "C:\\teste\\hand-position.csv", "w");
				if (error_code != 0) {
					printf("Error! Failed to open file in r mode!");
				}
				else {
					printf("I opened myfile.c in r mode!\n");
				}
				for (uint32_t h = 0; h < frame->nHands; h++) {
					LEAP_HAND* hand = &frame->pHands[h];
					save_hand_position(hand, file);
				}
				fclose(file);
				if (frame->nHands == 0) {
					if (std::remove("C:\\teste\\hand-position.csv") == 0) { printf("deu"); };
				}

			}
			//Para as imagens
			LEAP_IMAGE_EVENT* imageEvent = GetImage();
			if (imageEvent && (imageEvent->info.frame_id > lastImageID)) {
				lastImageID = imageEvent->info.frame_id;
			}

			// Send data to the client
			//std::cout << "Pré" << lastImageID << std::endl;
			std::string numberString = std::to_string(lastImageID);
			//std::cout <<"Meio" << numberString << std::endl;
			const char* charPtr = numberString.c_str();
			//std::cout << "Pós" << charPtr << std::endl;
			if (send(client_socket, charPtr, strlen(charPtr), 0) == SOCKET_ERROR) {
				std::cerr << "Failed to send data to client." << std::endl;
				closesocket(client_socket);
				closesocket(server_socket);
				WSACleanup();
				return 1;
			}
		}
	}
    // Close the client and server sockets
    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();
    return 0;
}
void save_hand_position(const LEAP_HAND* hand, FILE* file) {
	fprintf(file, "palm.position.x,palm.position.z");
	fprintf(file, "\n%f,%f", hand->palm.position.x, hand->palm.position.z);
}

void save_hand_data_csv2(const LEAP_HAND* hand, FILE* file) {
	fprintf(file, "grab_strength,palm.dir.x,palm.dir.y,palm.dir.z,palm.position.x,palm.position.y,palm.position.z,thumb.distal.rot.x,thumb.distal.rot.y,thumb.distal.rot.z,middle.distal.rot.x,middle.distal.rot.y,middle.distal.rot.z,thumb.distal.next.x,thumb.distal.next.y,thumb.distal.next.z,middle.distal.next.x,middle.distal.next.y,middle.distal.next.z");
	//Basic General Info - Verificar hand type
	fprintf(file, "\n%f,", hand->grab_strength);
	//Palm Information
	fprintf(file, "%f,%f,%f,%f,%f,%f,",
		hand->palm.direction.x,
		hand->palm.direction.y,
		hand->palm.direction.z,
		hand->palm.position.x,
		hand->palm.position.y,
		hand->palm.position.z);
	//Distal bone
	fprintf(file, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f",
			hand->digits[0].distal.rotation.x,
			hand->digits[0].distal.rotation.y,
			hand->digits[0].distal.rotation.z,
			hand->digits[2].distal.rotation.x,
			hand->digits[2].distal.rotation.y,
			hand->digits[2].distal.rotation.z,
			hand->digits[0].distal.next_joint.x,
			hand->digits[0].distal.next_joint.y,
			hand->digits[0].distal.next_joint.z,
			hand->digits[2].distal.next_joint.x,
			hand->digits[2].distal.next_joint.y,
			hand->digits[2].distal.next_joint.z);
}

void save_hand_data_csv(const LEAP_HAND* hand, FILE* file) {
	fprintf(file, "visible_time,grab_angle,grab_strength,pinch_dis,pinch_strength,left/right,palm.dir.x,palm.dir.y,palm.dir.z,palm.normal.x,palm.normal.y,palm.normal.z,palm.orienta.x,palm.orienta.y,palm.orienta.z,palm.orienta.w,palm.position.x,palm.position.y,palm.position.z,palm.velocity.x,palm.velocity.y,palm.velocity.z,palm.width,thumb_extend,index_extend,middle_extended,ring_extended,pinky_extend,arm.rot.x,arm.rot.y,arm.rot.z,arm.rot.w,arm.width,arm.prev.x,arm.prev.y,arm.prev.z,arm.next.x,arm.next.y,arm.next.z,thumb.metacarpal.rot.x,thumb.metacarpal.rot.y,thumb.metacarpal.rot.z,thumb.metacarpal.rot.w,thumb.metacarpal.width,thumb.metacarpal.prev.x,thumb.metacarpal.prev.y,thumb.metacarpal.prev.z,thumb.metacarpal.next.x,thumb.metacarpal.next.y,thumb.metacarpal.next.z,thumb.proximal.rot.x,thumb.proximal.rot.y,thumb.proximal.rot.z,thumb.proximal.rot.w,thumb.proximal.width,thumb.proximal.prev.x,thumb.proximal.prev.y,thumb.proximal.prev.z,thumb.proximal.next.x,thumb.proximal.next.y,thumb.proximal.next.z,thumb.intermediate.rot.x,thumb.intermediate.rot.y,thumb.intermediate.rot.z,thumb.intermediate.rot.w,thumb.intermediate.width,thumb.intermediate.prev.x,thumb.intermediate.prev.y,thumb.intermediate.prev.z,thumb.intermediate.next.x,thumb.intermediate.next.y,thumb.intermediate.next.z,thumb.distal.rot.x,thumb.distal.rot.y,thumb.distal.rot.z,thumb.distal.rot.w,thumb.distal.width,thumb.distal.prev.x,thumb.distal.prev.y,thumb.distal.prev.z,thumb.distal.next.x,thumb.distal.next.y,thumb.distal.next.z,index.metacarpal.rot.x,index.metacarpal.rot.y,index.metacarpal.rot.z,index.metacarpal.rot.w, index.metacarpal.width, index.metacarpal.prev.x, index.metacarpal.prev.y, index.metacarpal.prev.z, index.metacarpal.next.x, index.metacarpal.next.y, index.metacarpal.next.z, index.proximal.rot.x, index.proximal.rot.y, index.proximal.rot.z, index.proximal.rot.w, index.proximal.width, index.proximal.prev.x, index.proximal.prev.y, index.proximal.prev.z, index.proximal.next.x, index.proximal.next.y, index.proximal.next.z, index.intermediate.rot.x, index.intermediate.rot.y, index.intermediate.rot.z, index.intermediate.rot.w, index.intermediate.width, index.intermediate.prev.x, index.intermediate.prev.y, index.intermediate.prev.z, index.intermediate.next.x, index.intermediate.next.y, index.intermediate.next.z, index.distal.rot.x, index.distal.rot.y, index.distal.rot.z, index.distal.rot.w, index.distal.width, index.distal.prev.x, index.distal.prev.y, index.distal.prev.z, index.distal.next.x, index.distal.next.y, index.distal.next.z, middle.metacarpal.rot.x, middle.metacarpal.rot.y, middle.metacarpal.rot.z, middle.metacarpal.rot.w, middle.metacarpal.width, middle.metacarpal.prev.x, middle.metacarpal.prev.y, middle.metacarpal.prev.z, middle.metacarpal.next.x, middle.metacarpal.next.y, middle.metacarpal.next.z, middle.proximal.rot.x, middle.proximal.rot.y, middle.proximal.rot.z, middle.proximal.rot.w, middle.proximal.width, middle.proximal.prev.x, middle.proximal.prev.y, middle.proximal.prev.z, middle.proximal.next.x, middle.proximal.next.y, middle.proximal.next.z, middle.intermediate.rot.x, middle.intermediate.rot.y, middle.intermediate.rot.z, middle.intermediate.rot.w, middle.intermediate.width, middle.intermediate.prev.x, middle.intermediate.prev.y, middle.intermediate.prev.z, middle.intermediate.next.x, middle.intermediate.next.y, middle.intermediate.next.z, middle.distal.rot.x, middle.distal.rot.y, middle.distal.rot.z, middle.distal.rot.w, middle.distal.width, middle.distal.prev.x, middle.distal.prev.y, middle.distal.prev.z, middle.distal.next.x, middle.distal.next.y, middle.distal.next.z, ring.metacarpal.rot.x, ring.metacarpal.rot.y, ring.metacarpal.rot.z, ring.metacarpal.rot.w, ring.metacarpal.width, ring.metacarpal.prev.x, ring.metacarpal.prev.y, ring.metacarpal.prev.z, ring.metacarpal.next.x, ring.metacarpal.next.y, ring.metacarpal.next.z, ring.proximal.rot.x, ring.proximal.rot.y, ring.proximal.rot.z, ring.proximal.rot.w, ring.proximal.width, ring.proximal.prev.x, ring.proximal.prev.y, ring.proximal.prev.z, ring.proximal.next.x, ring.proximal.next.y, ring.proximal.next.z, ring.intermediate.rot.x, ring.intermediate.rot.y, ring.intermediate.rot.z, ring.intermediate.rot.w, ring.intermediate.width, ring.intermediate.prev.x, ring.intermediate.prev.y, ring.intermediate.prev.z, ring.intermediate.next.x, ring.intermediate.next.y, ring.intermediate.next.z, ring.distal.rot.x, ring.distal.rot.y, ring.distal.rot.z, ring.distal.rot.w, ring.distal.width, ring.distal.prev.x, ring.distal.prev.y, ring.distal.prev.z, ring.distal.next.x, ring.distal.next.y, ring.distal.next.z, pinky.metacarpal.rot.x, pinky.metacarpal.rot.y, pinky.metacarpal.rot.z, pinky.metacarpal.rot.w, pinky.metacarpal.width, pinky.metacarpal.prev.x, pinky.metacarpal.prev.y, pinky.metacarpal.prev.z, pinky.metacarpal.next.x, pinky.metacarpal.next.y, pinky.metacarpal.next.z, pinky.proximal.rot.x, pinky.proximal.rot.y, pinky.proximal.rot.z, pinky.proximal.rot.w, pinky.proximal.width, pinky.proximal.prev.x, pinky.proximal.prev.y, pinky.proximal.prev.z, pinky.proximal.next.x, pinky.proximal.next.y, pinky.proximal.next.z, pinky.intermediate.rot.x, pinky.intermediate.rot.y, pinky.intermediate.rot.z, pinky.intermediate.rot.w, pinky.intermediate.width, pinky.intermediate.prev.x, pinky.intermediate.prev.y, pinky.intermediate.prev.z, pinky.intermediate.next.x, pinky.intermediate.next.y, pinky.intermediate.next.z, pinky.distal.rot.x, pinky.distal.rot.y, pinky.distal.rot.z, pinky.distal.rot.w, pinky.distal.width, pinky.distal.prev.x, pinky.distal.prev.y, pinky.distal.prev.z, pinky.distal.next.x, pinky.distal.next.y, pinky.distal.next.z");
	//Basic General Info - Verificar hand type
	fprintf(file, "\n%d,%f,%f,%f,%f,%d,", hand->visible_time, hand->grab_angle, hand->grab_strength, hand->pinch_distance, hand->pinch_strength, hand->type == eLeapHandType_Left ? 1 : 0);
	//Palm Information
	fprintf(file, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,",
		hand->palm.direction.x,
		hand->palm.direction.y,
		hand->palm.direction.z,
		hand->palm.normal.x,
		hand->palm.normal.y,
		hand->palm.normal.z,
		hand->palm.orientation.x,
		hand->palm.orientation.y,
		hand->palm.orientation.z,
		hand->palm.orientation.w,
		hand->palm.position.x,
		hand->palm.position.y,
		hand->palm.position.z,
		hand->palm.velocity.x,
		hand->palm.velocity.y,
		hand->palm.velocity.z,
		hand->palm.width);
	//Extended info
	fprintf(file, "%d,%d,%d,%d,%d,", hand->thumb.is_extended, hand->index.is_extended, hand->middle.is_extended, hand->ring.is_extended, hand->pinky.is_extended);
	//Arm information
	fprintf(file, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f",
		hand->arm.rotation.x,
		hand->arm.rotation.y,
		hand->arm.rotation.z,
		hand->arm.rotation.w,
		hand->arm.prev_joint.x,
		hand->arm.prev_joint.y,
		hand->arm.prev_joint.z,
		hand->arm.next_joint.x,
		hand->arm.next_joint.y,
		hand->arm.next_joint.z,
		hand->arm.width);
	for (int i = 0; i < 5; i++) {
		//Metacarpal bone
		fprintf(file, ",%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,",
			hand->digits[i].metacarpal.rotation.x,
			hand->digits[i].metacarpal.rotation.y,
			hand->digits[i].metacarpal.rotation.z,
			hand->digits[i].metacarpal.rotation.w,
			hand->digits[i].metacarpal.width,
			hand->digits[i].metacarpal.prev_joint.x,
			hand->digits[i].metacarpal.prev_joint.y,
			hand->digits[i].metacarpal.prev_joint.z,
			hand->digits[i].metacarpal.next_joint.x,
			hand->digits[i].metacarpal.next_joint.y,
			hand->digits[i].metacarpal.next_joint.z);
		//Proximal bone
		fprintf(file, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,",
			hand->digits[i].proximal.rotation.x,
			hand->digits[i].proximal.rotation.y,
			hand->digits[i].proximal.rotation.z,
			hand->digits[i].proximal.rotation.w,
			hand->digits[i].proximal.width,
			hand->digits[i].proximal.prev_joint.x,
			hand->digits[i].proximal.prev_joint.y,
			hand->digits[i].proximal.prev_joint.z,
			hand->digits[i].proximal.next_joint.x,
			hand->digits[i].proximal.next_joint.y,
			hand->digits[i].proximal.next_joint.z);
		//Intermediate bone
		fprintf(file, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f",
			hand->digits[i].intermediate.rotation.x,
			hand->digits[i].intermediate.rotation.y,
			hand->digits[i].intermediate.rotation.z,
			hand->digits[i].intermediate.rotation.w,
			hand->digits[i].intermediate.width,
			hand->digits[i].intermediate.prev_joint.x,
			hand->digits[i].intermediate.prev_joint.y,
			hand->digits[i].intermediate.prev_joint.z,
			hand->digits[i].intermediate.next_joint.x,
			hand->digits[i].intermediate.next_joint.y,
			hand->digits[i].intermediate.next_joint.z);
		//Distal bone
		fprintf(file, ",%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f",
			hand->digits[i].distal.rotation.x,
			hand->digits[i].distal.rotation.y,
			hand->digits[i].distal.rotation.z,
			hand->digits[i].distal.rotation.w,
			hand->digits[i].distal.width,
			hand->digits[i].distal.prev_joint.x,
			hand->digits[i].distal.prev_joint.y,
			hand->digits[i].distal.prev_joint.z,
			hand->digits[i].distal.next_joint.x,
			hand->digits[i].distal.next_joint.y,
			hand->digits[i].distal.next_joint.z);
	}
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
