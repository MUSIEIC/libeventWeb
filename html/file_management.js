document.addEventListener('DOMContentLoaded', () => {
    const showListBtn = document.getElementById('showListBtn');
    const searchBtn = document.getElementById('searchBtn');
    const uploadForm = document.getElementById('uploadForm');
    const fileList = document.getElementById('fileList').getElementsByTagName('tbody')[0];
    const searchInput = document.getElementById('searchInput');

    // 显示文件列表
   showListBtn.addEventListener('click', async () => {
    try {
        const host = window.location.hostname; // 获取本机 IP 或域名
        const port = 12345; // 端口号保持固定
        const url = `http://${host}:${port}/api/files`;

        const response = await fetch(url);
        if (!response.ok) throw new Error('获取文件列表失败');
        const files = await response.json();
        renderFileList(files.files);
    } catch (error) {
        console.error('显示文件列表错误:', error);
    }
});
    // 搜索文件
    searchBtn.addEventListener('click', async () => {
        const keyword = searchInput.value.trim();
        if (!keyword) return;

        try {
            const host = window.location.hostname; // 获取本机 IP 或域名
            const port = 12345; // 端口号保持固定
            const url = `http://${host}:${port}/api/files/search?keyword=${encodeURIComponent(keyword)}`;
            const response = await fetch(url);
            // const response = await fetch(`http://127.0.0.1:12345/api/files/search?keyword=${encodeURIComponent(keyword)}`);
            if (!response.ok) throw new Error('搜索文件失败');
            const results = await response.json();
            renderFileList(results);
        } catch (error) {
            console.error('搜索文件错误:', error);
        }
    });

    // 文件上传
    uploadForm.addEventListener('submit', async (e) => {
        e.preventDefault();
        const fileInput = document.getElementById('uploadFile');
        const commentInput = document.getElementById('fileComment');
        const file = fileInput.files[0];
        const comment = commentInput.value.trim();

        if (!file) return;

        const formData = new FormData();
        formData.append('file', file);
        formData.append('comment', comment);

        try {
            const host = window.location.hostname; // 获取本机 IP 或域名
            const port = 12345; // 端口号保持固定
            const url = `http://${host}:${port}/api/files/upload`;
            const response = await fetch(url, {
                method: 'POST',
                body: formData
            });
            if (!response.ok) throw new Error('文件上传失败');
            alert('文件上传成功');
            uploadForm.reset();
        } catch (error) {
            console.error('文件上传错误:', error);
        }
    });

function renderFileList(files) {
    fileList.innerHTML = '';
    files.forEach(file => {
        const row = fileList.insertRow();
        row.insertCell().textContent = file.name;
        row.insertCell().textContent = file.location;
        row.insertCell().textContent = file.path;
        row.insertCell().textContent = file.type;
        row.insertCell().textContent = file.comment;

        const actionCell = row.insertCell();

        // 下载按钮
        const downloadBtn = document.createElement('button');
        downloadBtn.textContent = '下载';
        const host = window.location.hostname;
        const port = 12345;
        const url = `http://${host}:${port}/api/files/download?path=${encodeURIComponent(file.path)}`;
        downloadBtn.addEventListener('click', () => {
            window.open(url, '_blank');
        });
        actionCell.appendChild(downloadBtn);

        // 删除按钮
        const deleteBtn = document.createElement('button');
        deleteBtn.textContent = '删除';
        deleteBtn.style.marginLeft = '10px';
        deleteBtn.addEventListener('click', async () => {
            try {
                const response = await fetch(`http://${host}:${port}/api/files/delete?path=${encodeURIComponent(file.path)}`, {
                    method: 'POST'
                });

                if (!response.ok) throw new Error('删除失败');

                // 刷新文件列表
                showListBtn.click(); // 触发显示列表按钮的点击事件
            } catch (error) {
                console.error('删除文件错误:', error);
            }
        });

        actionCell.appendChild(deleteBtn);
    });
}
});