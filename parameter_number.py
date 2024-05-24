import torch.nn as nn
import re
# from torchvision.models import resnet50, ResNet50_Weights

import os
splitters = '[ \n]'
speed = 100*1024*1024 # 100 MB/s
for fname in os.listdir("./files/preload/"):
     folder_path = os.path.join("./files/preload/", fname)
     if os.path.isdir(folder_path):
        # print(fname)
        count_words = 0
        for file_name in os.listdir(folder_path):
            if "input" in file_name or ".txt" in file_name:
                continue
            file = open(os.path.join(folder_path, file_name), "r")
            line = file.readline()
            count_words += len(line.rstrip('\n').split(' '))
        # print(count_words)
        print(f"{fname}: {count_words} parameters, cost {count_words*4/speed} seconds")

# total_trainable_params = count_trainable_parameters(model)
# print(f"Total trainable parameters: {total_trainable_params}")
import torchvision.models as models

def count_parameters(model):
    return sum(p.numel() for p in model.parameters())

# Load the models
resnet18 = models.resnet18(pretrained=False)
vgg16 = models.vgg16(pretrained=False)
alexnet = models.alexnet(pretrained=False)

# Count and print the parameters
count_ResNet18 = count_parameters(resnet18)
print(f"ResNet-18: {count_ResNet18} parameters, cost {count_ResNet18*4/speed} seconds")
count_VGG16 = count_parameters(vgg16)
print(f"VGG16: {count_VGG16} parameters, cost {count_VGG16*4/speed} seconds")
count_AlexNet = count_parameters(alexnet)
print(f"AlexNet: {count_AlexNet} parameters, cost {count_AlexNet*4/speed} seconds")
# print(f"VGG16: {count_parameters(vgg16)} parameters")
# print(f"AlexNet: {count_parameters(alexnet)} parameters")