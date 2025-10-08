from sklearn.decomposition import PCA, KernelPCA


def get_princ_comps(data_set, n_components, method='kernel_pca'):
    pca = CustomPCA(data_set=data_set, n_components=n_components)
    if method == 'kernel_pca':
        return pca.get_pcs_kernel()
    elif method == 'pca':
        return pca.get_pcs()


class CustomPCA:
    def __init__(self, data_set, n_components):
        self.data_set = data_set
        self.n_components = n_components

    def get_pcs(self):
        pca = PCA(n_components=self.n_components, random_state=101)
        return pca.fit_transform(self.data_set)

    def get_pcs_kernel(self):
        pca = KernelPCA(n_components=self.n_components, random_state=101)
        return pca.fit_transform(self.data_set)
