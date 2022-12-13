# Training features

Large companies have large computing power, that allows them to set up many experiments. In this way,
for example, testing a large grid of hyperparameters, learning on very large datasets, and even selecting the 
optimal random seed. The vast majority of companies do not have such an opportunity, so they have to go the other way. 
We offer several techniques to improve the generalization of the model. It should improve your models as 
it improves ours.

## 1. Fine-tune BatchNorm

It's feature allows you to fix your imperfect dataset distribution, which could contains outliers.
Our new approach fix it via running_mean and running_var parameters of BatchNorms.  

### Results

To show, that this feature works, we took face recognition task, especially [pretrained models form insightface repo](https://github.com/deepinsight/insightface/tree/master/recognition/arcface_torch#model-zoo).
For fine-tine we took R100 model, which was trained on glint and corresponding config. 

In first experiment we used ImageNet21k (100 random pictures per class) as out-of-domain data, and we got such results 
on IJB-C dataset:

|  Methods  | 1e-06 | 1e-05 | 0.0001 | 0.001 |  0.01 |  0.1  |
|-----------|-------|-------|--------|-------|-------|-------|
| baseline  | 90.58 | 95.88 | 97.32  | 98.19 | 98.73 | 99.24 |
| finetuned | 92.11 | 96.04 | 97.29  | 98.12 | 98.70 | 99.26 |

As you can see, TPRs on FPR 1e-6 and 1e-5 has increased.

In second experiment instead of real images we generated normally distributed tensors and added them to each training tensor 
(labels doesn't matter, because we don't train model). Results on IJB-C dataset:

|  Methods  | 1e-06 | 1e-05 | 0.0001 | 0.001 |  0.01 |  0.1  |
|-----------|-------|-------|--------|-------|-------|-------|
| baseline  | 90.58 | 95.88 | 97.32  | 98.19 | 98.73 | 99.24 |
| finetuned | 91.63 | 95.94 | 97.22  | 98.12 | 98.68 | 99.22 |

The results are modest then with ImageNet21k, but it also works. The main pros of this method - you can use it without 
searching and downloading new datasets.

Last experiment was with blurring some part of dataset. It doesn't increase metrics on public datasets, but for our previous
release model it works (a slight worse than in first experiment). Results on IJB-C dataset:

|  Methods  | 1e-06 | 1e-05 | 0.0001 | 0.001 |  0.01 |  0.1  |
|-----------|-------|-------|--------|-------|-------|-------|
| baseline  | 90.58 | 95.88 | 97.32  | 98.19 | 98.73 | 99.24 |
| finetuned | 90.49 | 95.89 | 97.30  | 98.15 | 98.72 | 99.23 |

Our released model (which was submitted to NIST) we fine-tuned on ImageNet21k (100 picture per class) + other out-of-domain
public datasets (such as OpenImages, FractalDB-1k, NABirds, etc.). In total, we added 5% such images.


### How to use it in your own training pipeline?

1. Totally freeze your backbone/feature extractor. You can make it by func `finetune_batchnorms.freeze_backbone`

2. Add one of noise type to your dataset (or you can mix it):
    - blur augmentation (you can make it by func `finetune_batchnorms.add_blur_to_transforms`)
    - normal distributed noise (you can make it by func `finetune_batchnorms.add_noise_to_batch`, witch expand batch)
    - out-of-domain data (for example, for face recognition you can use any images without faces)

3. Train your pipeline as normal for a few epochs (you can save the intermediate state of backbone, because 
   the running parameters changing fast)

4. Evaluate all saved models due training and select best one

5. [optional] Visualise changes of BatchNorm parameters via [visualisation_utils.py](bn_finetune/visualisation_utils.py)
You can find our example at [visualisation example](bn_finetune/visualisation_example.ipynb).
