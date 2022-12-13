import torch
from torch.utils.data import DataLoader, Dataset
import numpy as np
import matplotlib.pyplot as plt
from types import MethodType
from typing import List
from IPython.display import Markdown, display
import seaborn as sns
import warnings

__all__ = ['plot_bn_mean_var', 'plot_bn_distributions', 'plot_ds_bn_prob']

warnings.filterwarnings("ignore")

QUANTILE_EJECT = 0.05


def l2_norm(inp, axis=1):
    norm = torch.norm(inp, 2, axis, True)
    output = torch.div(inp, norm)
    return output


def set_except_bn_eval(model, global_values, global_log_probs, bn_ids=None):
    if bn_ids is None:
        bn_ids = list(range(len(list(model.modules()))))
    bn_id = 0
    for layer in model.modules():
        if 'model' in str(layer.__class__):
            continue
        if 'container' in str(layer.__class__):
            continue
        if 'batchnorm' in str(layer.__class__):
            if bn_id in bn_ids:
                if 'BatchNorm1d' in str(layer) or 'BatchNorm2d' in str(layer):
                    layer.forward = MethodType(lambda a, b:
                                               verbose_batchnorm_forward(a, b, global_values, global_log_probs), layer)
                    layer.normal = torch.distributions.normal.Normal(layer.running_mean, layer.running_var)
                else:
                    raise
            else:
                if 'BatchNorm1d' in str(layer):
                    layer.forward = MethodType(lambda a, b: super(torch.nn.BatchNorm1d, a).forward(b), layer)
                elif 'BatchNorm2d' in str(layer):
                    layer.forward = MethodType(lambda a, b: super(torch.nn.BatchNorm2d, a).forward(b), layer)
                else:
                    raise
            bn_id += 1


def get_bn_count(model):
    bn_id = 0
    for layer in model.modules():
        if 'batchnorm' in str(layer.__class__):
            bn_id += 1
    return bn_id


def verbose_batchnorm_forward(self, x, global_values, global_log_probs):
    with torch.no_grad():
        if len(x.shape) > 2:
            _x = x.detach().transpose(0, 1).reshape(x.size(1), -1).T
            log_prob = -self.normal.log_prob(_x)

            _shape = list(x.shape)
            _shape[0], _shape[1] = _shape[1], _shape[0]

            log_prob = log_prob.T.reshape(_shape).transpose(0, 1)
            global_log_probs.append(log_prob)

            n = x.numel() / x.size(1)
            denom = torch.sqrt(self.running_var) + self.eps
            out = x - self.running_mean[None, :, None, None]
            out /= denom[None, :, None, None]
            global_values.append(out.detach())
        else:
            global_log_probs.append(-self.normal.log_prob(x.detach()))

            denom = torch.sqrt(self.running_var) + self.eps
            out = x - self.running_mean
            out /= denom
            global_values.append(out.detach())

    out = super(self.__class__, self).forward(x)
    return out


def prepare_batch(tensors, device=None):
    assert isinstance(tensors, (list, tuple)) and len(tensors) == 4
    assert isinstance(tensors[0], torch.Tensor) and tensors[0].shape[0] >= 5
    assert isinstance(tensors[1], torch.Tensor) and tensors[1].shape[0] >= 5
    assert isinstance(tensors[2], torch.Tensor) and tensors[2].shape[0] >= 5
    assert isinstance(tensors[3], torch.Tensor) and tensors[3].shape[0] >= 5

    batch_names = ['random_target', 'first rand.group', 'second rand.group', 'random_noised']
    img_indexes = [x.shape[0] for x in tensors]
    batch = torch.cat(tensors)
    if device is not None:
        batch = batch.to(device)
    return batch_names, img_indexes, batch


def q_tube(data):
    data = data[~np.isnan(data)]
    t = np.quantile(data, QUANTILE_EJECT), np.quantile(data, 1 - QUANTILE_EJECT)
    data = data[np.where(np.logical_and(data >= t[0], data <= t[1]))]
    return data


def hist_bn_distribution(model, batch, img_indexes, batch_names, bn_ids, title):
    global_values = []
    global_log_probs = []
    set_except_bn_eval(model, global_values, global_log_probs, bn_ids=bn_ids)
    with torch.no_grad():
        _ = model(batch)

    fig, axs = plt.subplots(1, len(bn_ids), figsize=(25, 5))

    for j, d in enumerate(global_values):
        s = 0
        for i, nid in enumerate(img_indexes):
            _d = d[s:s + nid]
            if len(d.shape) > 2:
                _d = _d.transpose(0, 1).reshape(d.size(1), -1)
            data = q_tube(_d[0].cpu().numpy())
            sns.kdeplot(data, ax=axs[j], label=batch_names[i])

            s += nid
        axs[j].grid()

    axs[0].legend()
    for i, _bn_id in enumerate(bn_ids):
        axs[i].set_title(f'{_bn_id} BN layer')

    fig.suptitle(title, y=1.0)
    plt.show()


def hist_log_probs(model, batch, img_indexes, batch_names, bn_ids, title):
    global_values = []
    global_log_probs = []
    set_except_bn_eval(model, global_values, global_log_probs, bn_ids=bn_ids)
    with torch.no_grad():
        _ = model(batch)

    fig, axs = plt.subplots(1, len(bn_ids), figsize=(25, 5))

    for j, d in enumerate(global_log_probs):
        s = 0
        for i, nid in enumerate(img_indexes):
            _d = d[s:s + nid]
            if len(d.shape) > 2:
                _d = _d.transpose(0, 1).reshape(d.size(1), -1)
            data = q_tube(_d.mean(1).cpu().numpy())
            sns.kdeplot(data, ax=axs[j], label=batch_names[i])

            s += nid
        axs[j].grid()

    axs[0].legend()
    for i, _bn_id in enumerate(bn_ids):
        axs[i].set_title(f'{_bn_id} BN layer')

    fig.suptitle(title, y=1.0)
    plt.show()


def hist_dist_center(model, batch, img_indexes, batch_names, bn_ids, title, dist_type='cos'):
    assert dist_type in ['cos', 'euclidean']
    global_values = []
    global_log_probs = []
    set_except_bn_eval(model, global_values, global_log_probs, bn_ids=bn_ids)
    with torch.no_grad():
        _ = model(batch)

    fig, axs = plt.subplots(1, len(bn_ids), figsize=(25, 5))

    for j, d in enumerate(global_values):
        s = 0
        for i, nid in enumerate(img_indexes):
            _d = d[s:s + nid]
            _d = _d.reshape(_d.size(0), -1)
            if dist_type == 'cos':
                emb = torch.nn.functional.normalize(_d)
                c = torch.nn.functional.normalize(emb.mean(0), dim=0)
                dist = torch.mv(emb, c)
            elif dist_type == 'euclidean':
                c = _d.mean(0)
                dist = torch.norm(_d - c, dim=1)
            else:
                raise
            data = q_tube(dist.cpu().numpy())
            sns.kdeplot(data, ax=axs[j], label=batch_names[i])

            s += nid
        axs[j].grid()

    axs[0].legend()
    for i, _bn_id in enumerate(bn_ids):
        axs[i].set_title(f'{_bn_id} BN layer')

    fig.suptitle(title, y=1.0)
    plt.show()


def run_ds_plot_bn_prob(datasets, batch_size, model, bn_ids, title, ylogscale=False, device=None):
    colors = ['#1f77b4', '#ff7f0e']
    lstyles = ['-', '--']

    assert len(datasets) == 2
    for nblur, dataset in enumerate(datasets):
        if nblur == 0:
            blur_type = 'no noise'
        else:
            blur_type = 'noised images'

        assert isinstance(dataset, Dataset)
        dl = DataLoader(dataset,
                        batch_size=batch_size,
                        shuffle=True,
                        pin_memory=True,
                        drop_last=True,
                        num_workers=4)

        global_values = []
        global_log_probs = []
        set_except_bn_eval(model, global_values, global_log_probs, bn_ids=bn_ids)

        for j, (imgs, labels) in enumerate(dl):
            with torch.no_grad():
                if device is not None:
                    imgs = imgs.to(device)
                _ = model(imgs)

            for i in range(-len(bn_ids), 0):
                lp = global_log_probs[i]
                if len(lp.shape) > 2:
                    global_log_probs[i] = lp.transpose(0, 1) \
                        .reshape(lp.size(1), -1) \
                        .mean(1)
                else:
                    global_log_probs[i] = lp.mean(1)

        for i in range(len(bn_ids)):
            sns.kdeplot(torch.cat(global_log_probs[i::len(bn_ids)]).cpu().numpy(),
                        label=f'{bn_ids[i]} BN layer, {blur_type}',
                        color=colors[i],
                        linestyle=lstyles[nblur])

    plt.xscale('log')
    if ylogscale:
        plt.yscale('log')
    plt.grid()
    plt.legend()
    plt.title(title)
    plt.show()


def get_running_vals(model):
    weights = []
    biases = []
    means = []
    vars = []

    for m in model.modules():
        classname = m.__class__.__name__
        if classname.find('BatchNorm') == 0:  # not BN
            means.append(m.running_mean.cpu().numpy().flatten())
            vars.append(m.running_var.cpu().numpy().flatten())
            weights.append(m.weight.detach().cpu().numpy().flatten())
            biases.append(m.bias.detach().cpu().numpy().flatten())

    return {'means': means, 'vars': vars, 'weights': weights, 'biases': biases}


def get_score(ab):
    a, b = ab
    return np.linalg.norm(a - b) / np.linalg.norm(b)


def plot_bn_mean_var(model0, tuned_models: List[torch.nn.Module], tuned_titles: List[str]):
    """
    The resulting plot shows the change in values of the running statistics
    :param model0: model before tuning
    :param tuned_models: list of tuned models
    :param tuned_titles: titles for legend (relative to tuned_models)
    """
    model0_info = get_running_vals(model0)
    assert len(tuned_models) == len(tuned_titles)
    assert 1 <= len(tuned_models) <= 2
    markers = ['x', 'o']
    cells = []
    for i, (model_tuned, title_tuned) in enumerate(list(zip(tuned_models, tuned_titles))):
        model_tuned_info = get_running_vals(model_tuned)
        model_tuned_scores = {k: list(map(get_score, zip(model_tuned_info[k], model0_info[k])))
                              for k in model0_info.keys()}
        cells.append((model_tuned_scores, title_tuned, markers[i]))
    fig, ax1 = plt.subplots(1, figsize=(15, 5))
    fig.suptitle('BN parameter items of tuned models\n' + r'scatter of ||P1 - P0|| / ||P0||')

    for i, (scale_dict, name, m) in enumerate(cells):
        ax1.scatter(range(len(scale_dict['means'])), scale_dict['means'], marker=m, label='running_mean of ' + name,
                    c='#1f77b4')
        ax1.scatter(range(len(scale_dict['means'])), scale_dict['vars'], marker=m, label='running_var of ' + name,
                    c='#ff7f0e')
    ax1.grid()
    ax1.legend()


def _get_title(*args, text=''):
    def _header(text):
        return f'<h3>{text}</h3>'

    if all(args[0] == x for x in args):
        args = [args[0]]
    h3_title = _header("(" + '-'.join(['{}'] * len(args)) + ") " + text)
    return h3_title.format(*args)


def _display_md(text):
    display(Markdown(text))


def plot_bn_distributions(target_data, rand_group1, rand_group2, noised_data,
                          models: List[torch.nn.Module], titles: List[str], device=None):
    """
    Plots shows a distributions of batch values after it has gone through batch normalization
    :param target_data: tensors without noise
    :param rand_group1: tensors for any random group 1
    :param rand_group2: tensors for any random group 2
    :param noised_data: tensors with target noise
    :param models: list of models for analysis
    :param titles: list of titles to describe models
    :param device: device for locating tensors
    """
    tensors = target_data, rand_group1, rand_group2, noised_data
    batch_names, img_indexes, batch = prepare_batch(tensors, device=device)

    assert isinstance(models, list) and len(models) > 1
    assert isinstance(titles, list) and len(models) == len(titles)
    bn_count = get_bn_count(models[0])

    bn_ids = list(range(5))
    _display_md(_get_title(bn_ids[0], bn_ids[-1], text='batchnorm outputs'))
    for model, title in list(zip(models, titles)):
        hist_bn_distribution(model, batch, img_indexes, batch_names, bn_ids, title)

    bn_ids = list(range(bn_count - 5, bn_count))
    _display_md(_get_title(bn_ids[0], bn_ids[-1], text='batchnorm outputs'))
    for model, title in list(zip(models, titles)):
        hist_bn_distribution(model, batch, img_indexes, batch_names, bn_ids, title)

    bn_ids = list(range(5))
    _display_md(_get_title(bn_ids[0], bn_ids[-1], text='batchnorm log_prob'))
    for model, title in list(zip(models, titles)):
        hist_log_probs(model, batch, img_indexes, batch_names, bn_ids, title)

    bn_ids = list(range(bn_count - 5, bn_count))
    _display_md(_get_title(bn_ids[0], bn_ids[-1], text='batchnorm log_prob'))
    for model, title in list(zip(models, titles)):
        hist_log_probs(model, batch, img_indexes, batch_names, bn_ids, title)

    for dist_type in ['cos', 'euclidean']:
        h3_title = _get_title(bn_ids[0], bn_ids[-1], text='center-distributions for outputs')
        bn_ids = list(range(5))
        _display_md(h3_title + 'in {} distance'.format(dist_type))
        for model, title in list(zip(models, titles)):
            hist_dist_center(model, batch, img_indexes, batch_names, bn_ids, title, dist_type)

        bn_ids = list(range(bn_count - 5, bn_count))
        _display_md(h3_title + 'in {} distance'.format(dist_type))
        for model, title in list(zip(models, titles)):
            hist_dist_center(model, batch, img_indexes, batch_names, bn_ids, title, dist_type)


def plot_ds_bn_prob(datasets: List[Dataset], batch_size, models: List[torch.nn.Module], titles: List[str],
                    device=None):
    """
    Likelihood regarding batchnorms of clean and noised datasets
    :param datasets: two datasets (clean and noised) in torch format
    :param batch_size: batch_size for DataLoader
    :param models: list of models for analysis
    :param titles: list of titles to explain models
    :param device: device for locating tensors
    """
    bn_count = get_bn_count(models[0])
    for model, title in list(zip(models, titles)):
        bn_ids = list(range(bn_count - 1, bn_count))
        _display_md(_get_title(bn_ids[0], bn_ids[-1], text='batchnorm'))
        run_ds_plot_bn_prob(datasets, batch_size, model, bn_ids, title, ylogscale=False, device=device)

    for model, title in list(zip(models, titles)):
        bn_ids = list(range(bn_count - 2, bn_count - 1))
        _display_md(_get_title(bn_ids[0], bn_ids[-1], text='batchnorm'))
        run_ds_plot_bn_prob(datasets, batch_size, model, bn_ids, title, ylogscale=False, device=device)
