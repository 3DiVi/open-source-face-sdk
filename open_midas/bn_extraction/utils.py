import torch


def get_running_vals(model):
    """
    Get running values from every batch normalization layer of network.

    :param model: torch.nn.Module with at least one batch normalization layer.
    :return: dict of running means, variance, weights and biases for every BN layer.
    """
    weights = []
    biases = []
    means = []
    vars = []

    for m in model.modules():
        classname = m.__class__.__name__
        if classname.find('BatchNorm') == 0:  # not BN
            means.append(m.running_mean.detach().flatten())
            vars.append(m.running_var.detach().flatten())
            weights.append(m.weight.detach().flatten())
            biases.append(m.bias.detach().flatten())

    return {'means': means, 'vars': vars, 'weights': weights, 'biases': biases}


def make_nonzero_running_vars_mask(running, eps=1e-05):
    """
    Make lists of start, end indices for every BN layer and boolean mask of non-zero layers.
    During extraction of KLD all channels concatenated to one tensor and this function
    makes list of indices to map specific layer's channels.

    :param running: torch.nn.Module with at least one batch normalization layer.
    :param eps: a value added to the denominator for numerical stability.
    :return: list of ids for start of layer, end of layer, and boolean mask with 1 at non-zero layers.
    """
    cur_idx = 0
    id_starts = []
    id_ends = []
    masks = []
    for v in running["vars"]:
        id_starts.append(cur_idx)
        bn_len = len(v)
        id_ends.append(cur_idx + bn_len)
        cur_idx += bn_len
        masks.append((v >= eps).numpy())
    return id_starts, id_ends, masks


@torch.no_grad()
def kl_div_normal(means1, vars1, means2, vars2, eps=1e-05):
    """
    Calculate symmetric KL divergence for two distributions

    :param means1: Vector of means of distribution1.
    :param vars1: Vector of vars of distribution1.
    :param means2: Vector of means of distribution2.
    :param vars2: Vector of means of distribution2.
    :param eps: A value added to the denominator for numerical stability.
    :return: Vector of symmetric Kullback–Leibler divergencies.
    """
    a_var_e = vars1 + eps
    a_mean = means1

    b_var_e = vars2 + eps
    b_mean = means2

    with torch.no_grad():
        l1 = ((a_var_e + torch.pow(a_mean - b_mean, 2)) / (2 * b_var_e)) - 0.5
        l2 = ((b_var_e + torch.pow(b_mean - a_mean, 2)) / (2 * a_var_e)) - 0.5
        return l1 + l2


@torch.no_grad()
def make_feature_tensor_kld(bn_outputs, running, device="cpu"):
    """
    Calculate KL divergence between all channels of all batch normalization(BN) layers and running stats.

    :param bn_outputs: Inputs to BN layers extracted during network inference.
    :param running: List of running statistics from every BN layer from network.
    :param device: Device to make calculation ("cpu"/"cuda").
    :return: Tensor of KL divergencies [B*C], where B is batch size, C is number of channels from ALL BN layers.
    """
    with torch.no_grad():
        features = None

        for j, current_bn in enumerate(bn_outputs):
            if len(current_bn.shape) > 2:
                current_bn = current_bn.flatten(start_dim=2)
                cv, cm = torch.var_mean(current_bn, dim=-1)
            else:
                continue

            cv = cv.to(device)
            cm = cm.to(device)
            running_means = torch.Tensor(running["means"][j]).to(device)
            running_vars = torch.Tensor(running["vars"][j]).to(device)

            kld = kl_div_normal(cm, cv, running_means, running_vars, bn_n=j)
            if features is None:
                features = kld
            else:
                features = torch.cat((features, kld), dim=-1)
        return features
