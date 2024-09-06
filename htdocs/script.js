document.getElementById('contact-form').addEventListener('submit', function(event) {
    event.preventDefault();  // 阻止表单默认提交

    // 获取表单数据
    const name = document.getElementById('name').value;
    const email = document.getElementById('email').value;
    const message = document.getElementById('message').value;

    // 简单验证
    if (name && email && message) {
        alert('Thank you, ' + name + '! We have received your message.');
    } else {
        alert('Please fill out all fields.');
    }

    // 重置表单
    this.reset();
});
