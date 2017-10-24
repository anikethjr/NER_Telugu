import random
from random import shuffle

for i in range(10):
    train_fraction = 0.8
    test_fraction = 0.2
    # dev_fraction = 0
    random.seed(i)

    file = open('data/Gold_Data_Telugu/NER_data.txt','r+')
    lines = file.readlines()

    words = []
    postags = []
    tags = []
    sentences = []
    sentences_tags = []
    sentences_postags = []

    cur = []
    cur_pos = []
    cur_tag = []

    tag_dict = {}
    tag_dict['B-PERSON'] = []
    tag_dict['I-PERSON'] = []
    tag_dict['B-ORG'] = []
    tag_dict['I-ORG'] = []
    tag_dict['B-LOC'] = []
    tag_dict['I-LOC'] = []
    tag_dict['B-MISC'] = []
    tag_dict['I-MISC'] = []
    tag_dict['O'] = []

    cnt = 0
    for line in lines:
        line = line.strip()
        line = line.split("\t")
        word = line[0].strip()
        tag = line[2].strip()
        postag = line[3].strip()

        if tag!='B-PERSON' and tag!='I-PERSON' and tag!='B-ORG' and tag!='I-ORG' and tag!='B-LOC' and tag!='I-LOC' and tag!='O':
            if tag[0]=='B':
                tag = 'B-MISC'
            else:
                tag = 'I-MISC'

        words.append(word)
        postags.append(postag)
        tags.append(tag)
        tag_dict[tag].append(cnt)
        cnt = cnt + 1

        cur.append(word)
        cur_tag.append(tag)
        cur_pos.append(postag)

        if word=='.' or word=='?' or word=='!':
            sentences.append(cur)
            sentences_tags.append(cur_tag)
            sentences_postags.append(cur_pos)
            cur = []
            cur_tag = []
            cur_pos = []

    shuffle(tag_dict['B-PERSON'])
    shuffle(tag_dict['I-PERSON'])
    shuffle(tag_dict['B-ORG'])
    shuffle(tag_dict['I-ORG'])
    shuffle(tag_dict['B-LOC'])
    shuffle(tag_dict['I-LOC'])
    shuffle(tag_dict['B-MISC'])
    shuffle(tag_dict['I-MISC'])
    shuffle(tag_dict['O'])

    zipped = zip(sentences,sentences_tags,sentences_postags)
    shuffle(zipped)
    sentences[:],sentences_tags[:],sentences_postags[:] = zip(*zipped)

    train = open('data/Gold_Data_Telugu/train_sentences_' + str(i) + '.txt',"w+")
    test = open('data/Gold_Data_Telugu/test_sentences_' + str(i) + '.txt',"w+")
    # dev = open('data/Gold_Data_Telugu/dev_sentences_' + str(i) + '.txt',"w+")

    train_IOB = open('data/Gold_Data_Telugu/train_sentences_' + str(i) + '_IOB.txt',"w+")
    test_IOB = open('data/Gold_Data_Telugu/test_sentences_' + str(i) + '_IOB.txt',"w+")
    # dev_IOB = open('data/Gold_Data_Telugu/dev_sentences_' + str(i) + '_IOB.txt',"w+")

    for i in range(len(sentences)):
        if i<len(sentences)*train_fraction:
            for w,t,pos in zip(sentences[i],sentences_tags[i],sentences_postags[i]):
                if t != 'O':
                    train.write(w + " " + pos + " " + t[2:] + "\n")
                else:
                    train.write(w + " " + pos + " " + t + "\n")
                train_IOB.write(w + " " + pos + " " + t + "\n")
            train.write("\n")
            train_IOB.write("\n")
        # elif i<len(sentences)*(train_fraction+dev_fraction):
        #     for w,t,pos in zip(sentences[i],sentences_tags[i],sentences_postags[i]):
        #         if t != 'O':
        #             dev.write(w + " " + pos + " " + t[2:] + "\n")
        #         else:
        #             dev.write(w + " " + pos + " " + t + "\n")
        #         dev_IOB.write(w + " " + pos + " " + t + "\n")
        #     dev.write("\n")
        #     dev_IOB.write("\n")
        else:
            for w,t,pos in zip(sentences[i],sentences_tags[i],sentences_postags[i]):
                if t != 'O':
                    test.write(w + " " + pos + " " + t[2:] + "\n")
                else:
                    test.write(w + " " + pos + " " + t + "\n")
                test_IOB.write(w + " " + pos + " " + t + "\n")
            test.write("\n")
            test_IOB.write("\n")

    train = open('data/Gold_Data_Telugu/train_words.txt',"w+")
    test = open('data/Gold_Data_Telugu/test_words.txt',"w+")

    train_out = []
    test_out = []

    for tag in tag_dict.keys():
        indices = tag_dict[tag]
        w = []
        t = []
        pos = []
        for i in indices:
            w.append(words[i])
            t.append(tags[i])
            pos.append(postags[i])
        for i in range(len(w)):
            if i<len(w)*train_fraction:
                train_out.append(w[i]+" "+pos[i]+" "+t[i])
            else:
                test_out.append(w[i]+" "+pos[i]+" "+t[i])

    shuffle(train_out)
    shuffle(test_out)

    for t in train_out:
        train.write(t + "\n")

    for t in test_out:
        test.write(t + "\n")