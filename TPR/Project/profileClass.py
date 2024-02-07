from sklearn.preprocessing import StandardScaler
from sklearn.decomposition import PCA
from sklearn.neighbors import LocalOutlierFactor
from sklearn import svm
from sklearn.metrics import confusion_matrix, ConfusionMatrixDisplay
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt
import numpy as np
import argparse
from sklearn.ensemble import IsolationForest

parser=argparse.ArgumentParser()
parser.add_argument('-t', '--train', nargs='?',required=True, help='train input file')
parser.add_argument('-a', '--anomaly', nargs='?',required=True, help='anomaly detection input file')
parser.add_argument('-w', '--widths', nargs='*',required=False, help='observation window width',default=[120])             # delta = 5, 60/5=12 samples per minute, 12*10=120 samples per 10 minutes
parser.add_argument('-s', '--slide', nargs='?',required=False, help='observation windows slide value',default=12)       # delta = 5, 60/5=12 samples per minute, sliding 1min
args=parser.parse_args()

train=args.train
ano=args.anomaly
width= [int(w) for w in args.widths]
#transform list into string has [ width1, width2, ... widthn ]
width= str(width)
slide= str(args.slide)

# load datasets
train_features = np.loadtxt(train+'_features_w'+width+'_s'+slide)
anomaly_features = np.loadtxt(ano+'_features_w'+width+'_s'+slide)


# merge normal and anomaly dataset
percentage=0.50
p = int(len(train_features)*percentage)
left= int(len(train_features)*(1-percentage))
test_features = np.vstack((train_features[:p,:], anomaly_features))
test_labels = np.hstack((np.zeros(left), np.ones(len(anomaly_features))))

print('p: ', p)
print('left: ', left)
print('test_features: ', test_features.shape)
print('anom: ', anomaly_features.shape)
print('test_labels: ', test_labels.shape)


print('\n-- Anomaly Detection based on Principal Component Analysis --')

##divide train_features %
train_features = train_features[p:,:]

scaler = StandardScaler()
scaler.fit(train_features)

normalized_train_features = scaler.transform(train_features)
normalized_test_features = scaler.transform(test_features)

pca = PCA(n_components=2)
pca.fit(normalized_train_features)

pca_train = pca.transform(normalized_train_features)
pca_test = pca.transform(normalized_test_features)

plt.scatter(pca_train[:,0], pca_train[:,1], c='g')
#get first left samples from test_features
plt.scatter(pca_test[:left,0], pca_test[:left,1], c='b')
#get last samples from test_features
plt.scatter(pca_test[left:,0], pca_test[left:,1], c='r')


plt.title('PCA of Train and Test Data')
plt.legend(['Train','Test Normal','Test Anomaly'])
plt.show()

pca_ml = PCA(n_components=50)
pca_ml.fit(normalized_train_features)

train_features = pca_ml.transform(normalized_train_features)
test_features = pca_ml.transform(normalized_test_features)


print('\n-- Anomaly Detection based on One Class Support Vector Machines--')
ocsvm = svm.OneClassSVM(gamma='scale',kernel='linear',nu=0.5).fit(train_features)
poly_ocsvm = svm.OneClassSVM(gamma='scale',kernel='poly',nu=0.05, degree=3).fit(train_features)

rbf_ocsvm = svm.OneClassSVM(gamma='scale',kernel='rbf',nu=0.05).fit(train_features)
if_ocsvm = IsolationForest(max_samples='auto').fit(train_features)
lof_ocsvm = LocalOutlierFactor(novelty=True).fit(train_features)


L1=ocsvm.predict(test_features)
L2=rbf_ocsvm.predict(test_features)
L3=poly_ocsvm.predict(test_features)
L4=if_ocsvm.predict(test_features)
L5=lof_ocsvm.predict(test_features)

#L1 change 1 to 0 and -1 to 1
L1_res=np.where(L1==-1, 1, 0)
#L3 change 1 to 0 and -1 to 1
L3_res=np.where(L3==-1, 1, 0)
#L5 change 1 to 0 and -1 to 1
L5_res=np.where(L5==-1, 1, 0)

#L4 change 1 to 0 and -1 to 1
L4_res=np.where(L4==-1, 1, 0)
#L2 change 1 to 0 and -1 to 1
L2_res=np.where(L2==-1, 1, 0)

print('\n-- Confusion Matrix for Kernel Linear: --')
ConfusionMatrixDisplay(confusion_matrix=confusion_matrix(test_labels, L1_res)).plot()
plt.title('Confusion Matrix for Kernel Linear')
plt.show()

print('\n-- Confusion Matrix for Kernel Poly: --')
ConfusionMatrixDisplay(confusion_matrix=confusion_matrix(test_labels, L3_res)).plot()
plt.title('Confusion Matrix for Kernel Poly')
plt.show()

print('\n-- Confusion Matrix for Local Outlier Factor: --')
ConfusionMatrixDisplay(confusion_matrix=confusion_matrix(test_labels, L5_res)).plot()
plt.title('Confusion Matrix for Local Outlier Factor')
plt.show()

print('\n-- Confusion Matrix for Isolation Forest: --')
ConfusionMatrixDisplay(confusion_matrix=confusion_matrix(test_labels, L4_res)).plot()
plt.title('Confusion Matrix for Isolation Forest')
plt.show()

print('\n-- Confusion Matrix for Kernel RBF: --')
ConfusionMatrixDisplay(confusion_matrix=confusion_matrix(test_labels, L2_res)).plot()
plt.title('Confusion Matrix for Kernel RBF')
plt.show()

print('\n-- Ensemble --')
L1_binary = np.where(L1==-1, 1, 0)
L2_binary = np.where(L2==-1, 1, 0)
L3_binary = np.where(L3==-1, 1, 0)
L4_binary = np.where(L4==-1, 1, 0)
L5_binary = np.where(L5==-1, 1, 0)


print('\n-- Confusion Matrix for Bagging Ensemble: --')
votes = (0.33 * L2_binary) + (0.33 * L4_binary) + (0.33 * L5_binary)
ensemble_prediction = (votes > 0.49).astype(int)
ConfusionMatrixDisplay(confusion_matrix=confusion_matrix(test_labels, ensemble_prediction)).plot()
plt.title('Confusion Matrix for Bagging Ensemble')
plt.show()

print('\n-- Confusion Matrix for Bayes Ensemble: --')
weighted_votes = (0.245 * L2_binary) + (0.245 * L4_binary) + (0.51 * L5_binary)
ensemble_bayes_prediction = (weighted_votes > 0.49).astype(int)
ConfusionMatrixDisplay(confusion_matrix=confusion_matrix(test_labels, ensemble_bayes_prediction)).plot()
plt.title('Confusion Matrix for Bayes Ensemble')
plt.show()
