from django.db import models

class SEN0159(models.Model):
    id = models.AutoField(primary_key=True)
    co2 = models.FloatField()
    time = models.DateTimeField(auto_now_add=True)

    def __str__(self):
        return f'{self.id} {self.temperature} {self.humidity} {self.time}'

    class Meta:
        verbose_name = 'SEN0159'
        verbose_name_plural = 'SEN0159'
        ordering = ['id']