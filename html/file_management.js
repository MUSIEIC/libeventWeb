document.addEventListener('DOMContentLoaded', () => {
    const showListBtn = document.getElementById('showListBtn');
    const searchBtn = document.getElementById('searchBtn');
    const uploadForm = document.getElementById('uploadForm');
    const fileList = document.getElementById('fileList').getElementsByTagName('tbody')[0];
    const searchInput = document.getElementById('searchInput');

    // 显示文件列表
    showListBtn.addEventListener('click', async () => {
        try {
            const response = await fetch('http://127.0.0.1:12345/api/files');
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
            const response = await fetch(`http://127.0.0.1:12345/api/files/search?keyword=${encodeURIComponent(keyword)}`);
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
            const response = await fetch('http://127.0.0.1:12345/api/files/upload', {
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

    // 渲染文件列表
    function renderFileList(files) {
        fileList.innerHTML = '';
        files.forEach(file => {
            const row = fileList.insertRow();
            row.insertCell().textContent = file.name;
            row.insertCell().textContent = file.path;
            row.insertCell().textContent = file.type;
            row.insertCell().textContent = file.comment;
            
            const actionCell = row.insertCell();
            const downloadBtn = document.createElement('button');
            downloadBtn.textContent = '下载';
            downloadBtn.addEventListener('click', () => {
                window.open(`http://127.0.0.1:12345/api/files/download?path=${encodeURIComponent(file.path)}`, '_blank');
            });
            actionCell.appendChild(downloadBtn);
        });
    }
});