FROM centos:latest 

COPY crkit-master/* /crkit

#RUN /bin/bash -c "source /crkit/bin/crkit-env.sh"

RUN echo "source /crkit/bin/crkit-env.sh" >> ~/.bashrc

CMD /bin/bash
