import argparse
import numpy as np
import scipy.stats as stats
import matplotlib.pyplot as plt
import scalogram

def extratctSilenceActivity(data,threshold=0):
    if(data[0]<=threshold):
        s=[1]
        a=[]
    else:
        s=[]
        a=[1]
    for i in range(1,len(data)):
        if(data[i-1]>threshold and data[i]<=threshold):
            s.append(1)
        elif(data[i-1]<=threshold and data[i]>threshold):
            a.append(1)
        elif (data[i-1]<=threshold and data[i]<=threshold):
            s[-1]+=1
        else:
            a[-1]+=1
    return(s,a)

def extractTimeFeatures(data, threshold=0):
    silence,activity=extratctSilenceActivity(data,threshold)
    if len(silence)>0:
        silence_faux=np.array([len(silence),np.mean(silence),np.std(silence), np.var(silence)])
    else:
        silence_faux=np.zeros(4)
        
    # if len(activity)>0:
        # activity_faux=np.array([len(activity),np.mean(activity),np.std(activity)])
    # else:
        # activity_faux=np.zeros(3)
    # activity_features=np.hstack((activity_features,activity_faux))  
    
    features=np.hstack((silence_faux))
    return(features)

def extractPeriodicityFeatures(data):
    scales=np.arange(2,50)                  
    S,scales=scalogram.scalogramCWT(data,scales)   #periodogram using CWT (Morlet wavelet)
    #replace NaN with 0
    S=np.nan_to_num(S)
    return S

def extractCorrelationFeatures(data):
    return np.corrcoef(data)

def extractCovarianceFeatures(data):
    return np.cov(data)

def extractStats(data):
    nSamp=data.shape
    #print(data)

    M1=np.mean(data,axis=0)
    Md1=np.median(data,axis=0)
    Std1=np.std(data,axis=0)
    Var = np.var(data,axis=0)

    features=np.hstack((M1,Md1,Std1,Var))

    return(features)

def slidingMultObsWindow(data,allLengthObsWindow,slidingValue, type=0, threshold=0):
    iobs=0
    nSamples,nMetrics=data.shape
    while iobs*slidingValue<nSamples-max(allLengthObsWindow):
        obsFeatures=np.array([])
        for lengthObsWindow in allLengthObsWindow:
            for m in np.arange(nMetrics):
                if type==0:
                    wmFeatures=extractStats(data[iobs*slidingValue:iobs*slidingValue+lengthObsWindow,m])
                elif type==1:
                    wmFeatures=extractTimeFeatures(data[iobs*slidingValue:iobs*slidingValue+lengthObsWindow,m], threshold)
                elif type==2:
                    wmFeatures=extractPeriodicityFeatures(data[iobs*slidingValue:iobs*slidingValue+lengthObsWindow,m])
                elif type==3:
                    wmFeatures=extractCorrelationFeatures(data[iobs*slidingValue:iobs*slidingValue+lengthObsWindow,m])
                elif type==4:
                    wmFeatures=extractCovarianceFeatures(data[iobs*slidingValue:iobs*slidingValue+lengthObsWindow,m])
            
                obsFeatures=np.hstack((obsFeatures,wmFeatures))
            iobs+=1
        
        if 'allFeatures' not in locals():
            allFeatures=obsFeatures.copy()
        else:
            allFeatures=np.vstack((allFeatures,obsFeatures))

    return(allFeatures)

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('-i', '--input', nargs='?',required=True, help='input file')
    parser.add_argument('-w', '--widths', nargs='*',required=False, help='list of observation window width',default=[120])    # delta = 5, 60/5=12 samples per minute, 12*5=60 samples per 5 minutes
    parser.add_argument('-s', '--slide', nargs='?',required=False, help='observation windows slide value',default=12)       # delta = 5, 60/5=12 samples per minute, sliding 1min
    parser.add_argument('-th', '--treshold', nargs='?',required=False, help='anomaly detection treshold',default=0)
    args=parser.parse_args()
    
    fileInput=args.input
    lengthObsWindow=[int(w) for w in args.widths]
    slidingValue=int(args.slide)
    silence_treshold=int(args.treshold)
        
    data=np.loadtxt(fileInput,dtype=float)
    fname=''.join(fileInput.split('.')[:-1])+"_features_w{}_s{}".format(lengthObsWindow,slidingValue)

    print("### SLIDING Observation Windows with Lengths {} and Sliding {} ###".format(lengthObsWindow,slidingValue))    
    
    # Number of DNS Requests / DNS Responses
    packets = np.column_stack((data[:,0], data[:,2]))#, data[:, 0] + data[:, 2]))
    features_1 = slidingMultObsWindow(packets, lengthObsWindow, slidingValue)

    # Ratio DNS Query/Upload Bytes and DNS Reply/Download Bytes
    racio = np.column_stack((data[:,1]/data[:,0], data[:,3]/data[:,2]))
    racio = np.nan_to_num(racio)
    features_2 = slidingMultObsWindow(racio, lengthObsWindow, slidingValue)

    # Average Time DNS Query/Reply / DNS Queries / DNS Packets / Min Delta / Max Delta
    time_feat = data[:,-5:]
    features_3 = slidingMultObsWindow(time_feat, lengthObsWindow, slidingValue)
    
    # periodicity of avg time between 2 dns requests
    p_avg_request_delta = data[:, -4].reshape(-1, 1)
    features_avg_request_delta = slidingMultObsWindow(p_avg_request_delta, lengthObsWindow, slidingValue, type=2)
    # periodicity of avg time between all dns packets
    p_avg_delta = data[:, -3].reshape(-1, 1)
    features_avg_delta = slidingMultObsWindow(p_avg_delta, lengthObsWindow, slidingValue, type=2)
    # periodicity of max delta
    p_max_delta = data[:, -1].reshape(-1, 1)
    features_max_delta = slidingMultObsWindow(p_max_delta, lengthObsWindow, slidingValue, type=2)

    periodicities = np.column_stack((features_avg_request_delta, features_avg_delta, features_max_delta))

    # covariance
    # DNS Queries and upload bytes -> 0
    cov0 = np.column_stack((data[:, 0], data[:, 1]))
    features_cov0 = slidingMultObsWindow(cov0, lengthObsWindow, slidingValue, type=4)

    # min and max delta -> 2
    cov2 = np.column_stack((data[:, -2], data[:, -1]))
    features_cov2 = slidingMultObsWindow(cov2, lengthObsWindow, slidingValue, type=4)

    #print shaès
    print("Feature_1: {}".format(features_1.shape))
    print("Feature_2: {}".format(features_2.shape))
    print("Feature_3: {}".format(features_3.shape))
    print("Feature_avg_response_delta: {}".format(periodicities.shape))
    print("Feature_cov: {}".format(np.column_stack((features_cov0, features_cov2)).shape))

    #features_1,features_time,features_4,features_2, np.column_stack((features_avg_response_delta, features_avg_request_delta, features_avg_delta, features_max_delta)) -> concatenate
    features=np.hstack((features_1,features_2,features_3, periodicities, np.column_stack((features_cov0, features_cov2))))

    np.savetxt(fname, features, fmt='%f')


if __name__ == '__main__':
    main()
