from model.ner_model import NERModel
from model.config import Config

def predict_test_sansIOB(model):
    test_file = open(config.filename_test[:-4] + "_IOB.txt","r")
    out_file = open(config.filename_predictions,"w+")    
    sentence = []
    orig_tags = []
    lines = test_file.readlines()
    for line in lines:
        line = line.strip().split(' ')
        if len(line) == 1:
            pred_tags = model.predict(sentence) 
            i = 0           
            while i < len(pred_tags)-1:
                if pred_tags[i]!='O':
                    beg = i
                    i = i+1
                    while(i<len(pred_tags)-1 and pred_tags[i]==pred_tags[beg]):
                        i = i+1
                    end = i-1
                    pred_tags[beg] = "B-" + pred_tags[beg]
                    for j in range(beg+1,end+1):
                        pred_tags[j] = "I-" + pred_tags[j]
                else:
                    i = i+1
            for word,tag,otag in zip(sentence,pred_tags,orig_tags):
                out_file.write(word + " " + otag + " " + tag + "\n")
            out_file.write("\n")
            sentence = []
            orig_tags = []
        else:
            sentence.append(line[0])   
            orig_tags.append(line[-1])

def predict_test(model):
    test_file = open(config.filename_test,"r")
    out_file = open(config.filename_predictions,"w+")    
    sentence = []
    orig_tags = []
    lines = test_file.readlines()
    for line in lines:
        line = line.strip().split(' ')
        if len(line) == 1:
            pred_tags = model.predict(sentence) 
            for word,tag,otag in zip(sentence,pred_tags,orig_tags):
                out_file.write(word + " " + otag + " " + tag + "\n")
            out_file.write("\n")
            sentence = []
            orig_tags = []
        else:
            sentence.append(line[0])   
            orig_tags.append(line[-1])


# create instance of config
config = Config()

# build model
model = NERModel(config)
model.build()
model.restore_session(config.dir_model)

# predict output on test set
predict_test(model)