# Named Entity Recognition for Telugu using LSTM-CRF

The code for the paper titled "Named Entity Recognition for Telugu using LSTM-CRF".

The dataset can be found in the data/Gold_Data_Telugu folder. The code for reproducing the results is in the lstmcrf folder.

Steps to reproduce LSTM-CRF results:

1. Run the build_data.py file which generates the vocabulary and the directory structure

2. Train the model by running train.py

3. Get the model's predictions on the test set by executing predict_test.py

4. Run the evaluation script in the conll_evaluation folder by executing "perl conll < ../data/LSTM-CRF/predictions/predictions_9-no-dev.txt". The values of the various metrics will be displayed.

Steps to reproduce YamCha results:

1. Run "./configure"

2. Run "make"

3. Execute "sudo make install"

4. Execute 'make CORPUS=../data/Gold_Data_Telugu/train_sentences_9_IOB.txt MODEL=mon_project train SVM_PARAM="-t1 -d2 -c1" train' in the yamcha folder to train the model.

5. Execute 'yamcha -m mon_project.model < ../data/Gold_Data_Telugu/test_sentences_9_IOB.txt > ../data/YamCha/results9_IOB.txt' to get the predictions of the model of the test set.

6. Run the evaluation script in the conll_evaluation folder by executing "perl conll < ../data/YamCha/results9_IOB.txt". The values of the various metrics will be displayed.

Steps to reproduce CRF++ results:

1. Run "./configure"

2. Run "make"

3. Execute "sudo make install"

4. To train the model, run "./crf_learn -f 3 -c 1.5 template ../data/Gold_Data_Telugu/train_sentences_9_IOB.txt model"

5. To get the predictions of the model of the test set, run "./crf_test -m model ../data/Gold_Data_Telugu/test_sentences_9_IOB.txt > ../data/CRF++/results9_IOB.data"

6. Run the evaluation script in the conll_evaluation folder by executing "perl conll < ../data/CRF++/results9_IOB.data". The values of the various metrics will be displayed.
