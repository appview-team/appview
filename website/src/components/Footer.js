import React from "react";
import { Container, Row, Col } from "react-bootstrap";
import logo from "../images/appview-invert.png";
import "../scss/_footer.scss";
import "../utils/font-awesome";

export default function Footer() {
  return (
    <Container fluid className="footer-container ">
      <Container>
        <Row>
          <Col xs={12} md={6} className="text-left footer-left">
            <a href="https://appview.org">
              <img src={logo} alt="AppView" width={125} />
            </a>
          </Col>
          <Col xs={12} md={6} className="text-right footer-right">
            <p>&copy;2024 AppView | <a href="https://github.com/appview-team/appview/blob/master/LICENSE" target="_blank" rel="noreferrer noopener"><strong>License</strong></a></p>
          </Col>
        </Row>
      </Container>
    </Container>
  );
}
